/*********************************************************************
 * \file   ProfileStorage.cpp
 * \brief  Implementation of ProfileStorage class and utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "ProfileStorage.hpp"

namespace senc::clientapi::storage
{
	ProfileStorageIterator::ProfileStorageIterator(const ProfileEncKey& key,
												   ProfileInputFile& file,
												   utils::file_pos_t pos)
		: _key(key), _file(file), _pos(pos),
		  _recordEncSize1(read_profile_record_enc_size(_file)),
		  _recordEncSize2(read_profile_record_enc_size(_file)),
		  _record(read_profile_record(_file, _key, _recordEncSize1, _recordEncSize2)) { }

	bool ProfileStorageIterator::operator==(const Self& other) const
	{
		return (this->_pos == other._pos);
	}

	ProfileStorageIterator::Self& ProfileStorageIterator::operator++()
	{
		this->_pos = next_pos();
		this->_file.get().set_pos(this->_pos);
		this->_recordEncSize1 = read_profile_record_enc_size(_file);
		this->_recordEncSize2 = read_profile_record_enc_size(_file);
		this->_record = read_profile_record(_file, _key, _recordEncSize1, _recordEncSize2);
	}

	ProfileStorageIterator::Self ProfileStorageIterator::operator++(int)
	{
		return Self(_key, _file, next_pos());
	}

	ProfileStorageIterator::reference ProfileStorageIterator::operator*() const
	{
		return _record;
	}

	ProfileStorageIterator::pointer ProfileStorageIterator::operator->() const
	{
		return &_record;
	}

	utils::file_pos_t ProfileStorageIterator::next_pos() const
	{
		// next record starts after sizes and record ciphertext
		return this->_pos + (2 * sizeof(profile_record_enc_size_t)) + _recordEncSize1 + _recordEncSize2;;
	}

	profile_record_enc_size_t ProfileStorageIterator::read_profile_record_enc_size(ProfileInputFile& file)
	{
		return file.read<profile_record_enc_size_t>();
	}

	ProfileRecord ProfileStorageIterator::read_profile_record(ProfileInputFile& file,
															  const ProfileEncKey& key,
															  profile_record_enc_size_t size1,
															  profile_record_enc_size_t size2)
	{
		// read encrypted profile
		ProfileEncCiphertext encProfile{};
		auto& [enc1, enc2] = encProfile;
		enc1.resize(size1);
		enc2.resize(size2);
		file.read(enc1.data(), size1);
		file.read(enc2.data(), size2);

		// decrypt
		utils::Buffer profileBytes = schema().decrypt(encProfile, key);

		// parse
		return parse_profile_record(key, profileBytes);
	}

	ProfileRecord ProfileStorageIterator::parse_profile_record(const ProfileEncKey& key, utils::Buffer& data)
	{
		const auto end = data.end();
		auto it = data.begin();

		utils::byte flagsByte = 0;
		it = utils::read_bytes(flagsByte, it, end);
		const auto flags = ProfileRecordFlags::from_byte(flagsByte);

		UserSetID usersetID{};
		it = utils::read_bytes(usersetID, it, end);

		PubKey regLayerPubKey{}, ownerLayerPubKey{};
		{
			utils::Buffer pubKeyBuff(PubKey::ENCODED_SIZE);
			it = utils::read_bytes(pubKeyBuff, it, end);
			regLayerPubKey = PubKey::decode(pubKeyBuff);
			it = utils::read_bytes(pubKeyBuff, it, end);
			ownerLayerPubKey = PubKey::decode(pubKeyBuff);
		}

		// this constant is true as shard IDs are currently sampled from [1,MAX_MEMBER_COUNT]
		// TODO: move this constant elsewhere in future version
		constexpr std::size_t SHARD_ID_MAX_SIZE = sizeof(member_count_t);

		// this constant is true as shard values are always < modulus
		// TODO: move this constant elsewhere in future version
		const std::size_t SHARD_VALUE_MAX_SIZE = (PrivKeyShardValue::modulus() - 1).MinEncodedSize();

		PrivKeyShard regLayerPrivKeyShard{};
		utils::Buffer shardIdBuff(SHARD_ID_MAX_SIZE);
		utils::Buffer shardValBuff(SHARD_VALUE_MAX_SIZE);
		utils::BigInt shardValUnderlying{};
		it = utils::read_bytes(shardIdBuff, it, end);
		it = utils::read_bytes(shardValBuff, it, end);
		regLayerPrivKeyShard.first.Decode(shardIdBuff.data(), shardIdBuff.size());
		shardValUnderlying.Decode(shardValBuff.data(), shardValBuff.size());
		regLayerPrivKeyShard.second = std::move(shardValUnderlying);

		// if non-owner record, stop here - no more data to read
		if (!flags.is_owner)
			return ProfileRecord::reg(
				std::move(usersetID),
				std::move(regLayerPubKey), std::move(ownerLayerPubKey),
				std::move(regLayerPrivKeyShard)
			);

		// otherwise, read remaining data (owner shard) then return
		PrivKeyShard ownerLayerPrivKeyShard{};
		it = utils::read_bytes(shardIdBuff, it, end);
		it = utils::read_bytes(shardValBuff, it, end);
		ownerLayerPrivKeyShard.first.Decode(shardIdBuff.data(), shardIdBuff.size());
		shardValUnderlying.Decode(shardValBuff.data(), shardValBuff.size());
		ownerLayerPrivKeyShard.second = std::move(shardValUnderlying);

		return ProfileRecord::owner(
			std::move(usersetID),
			std::move(regLayerPubKey), std::move(ownerLayerPubKey),
			std::move(regLayerPrivKeyShard), std::move(ownerLayerPrivKeyShard)
		);
	}

	ProfileEncSchema& ProfileStorageIterator::schema()
	{
		static thread_local ProfileEncSchema schema;
		return schema;
	}

	ProfileStorage::ProfileStorage(const std::string& path,
								   const std::string& username,
								   const std::string& password)
		: _path(path), _key(derive_key(username, password)) { }

	ProfileEncKey ProfileStorage::derive_key(const std::string& username, const std::string& password)
	{
		using KDF = utils::pwd::PBKDF2<utils::enc::AES1L::KEY_SIZE>;
		static thread_local KDF kdf;

		// generate salt from username as such: going backwards, copy username 
		// repeatedly until filled entire salt
		KDF::Salt salt{};
		if (!username.empty())
			for (std::size_t i = 0; i < salt.size(); ++i)
				salt[i] = username[username.length() - (i % username.length()) - 1];

		auto bytes = kdf.hash(password, salt);
		return utils::enc::AES1L::Key(bytes.data(), bytes.size());
	}
}
