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
	const std::size_t ProfileUtils::SHARD_VALUE_MAX_SIZE = (PrivKeyShardValue::modulus() - 1).MinEncodedSize();

	profile_record_enc_sizes_t ProfileUtils::read_profile_record_enc_sizes(ProfileInputFile& file)
	{
		profile_record_enc_sizes_t res{};
		file.read(&res.first, 1);
		file.read(&res.second, 1);
		return res;
	}

	ProfileRecord ProfileUtils::read_profile_record(ProfileInputFile& file,
													const ProfileEncKey& key,
													profile_record_enc_sizes_t sizes)
	{
		// read encrypted profile
		ProfileEncCiphertext encProfile{};
		auto& [enc1, enc2] = encProfile;
		enc1.resize(sizes.first);
		enc2.resize(sizes.second);
		file.read(enc1.data(), sizes.first);
		file.read(enc2.data(), sizes.second);

		// decrypt
		utils::Buffer profileBytes = schema().decrypt(encProfile, key);

		// parse
		return parse_profile_record(profileBytes);
	}

	void ProfileUtils::write_profile_record_with_enc_sizes(ProfileOutputFile& file,
														   const ProfileEncKey& key,
														   const ProfileRecord& record)
	{
		auto profileBytes = serialize_profile_record(record);
		const auto [enc1, enc2] = schema().encrypt(profileBytes, key);
		auto sizes = profile_record_enc_sizes_t{ enc1.size(), enc2.size() };

		file.append(sizes.first);
		file.append(sizes.second);

		file.append(enc1.data(), enc1.size());
		file.append(enc2.data(), enc2.size());
	}

	ProfileEncSchema& ProfileUtils::schema()
	{
		static thread_local ProfileEncSchema schema;
		return schema;
	}

	ProfileRecord ProfileUtils::parse_profile_record(utils::Buffer& data)
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

	utils::Buffer ProfileUtils::serialize_profile_record(const ProfileRecord& record)
	{
		utils::Buffer res{};

		ProfileRecordFlags flags{
			.is_owner = record.is_owner()
		};
		const auto flagsByte = flags.to_byte();
		utils::write_bytes(res, flagsByte);

		utils::write_bytes(res, record.reg_layer_pub_key().encode());
		utils::write_bytes(res, record.owner_layer_pub_key().encode());

		utils::BigInt shardValUnderlying{};
		res.resize(res.size() + SHARD_ID_MAX_SIZE);
		record.reg_layer_priv_key_shard().first.Encode(res.data() - SHARD_ID_MAX_SIZE, SHARD_ID_MAX_SIZE);
		res.resize(res.size() + SHARD_VALUE_MAX_SIZE);
		shardValUnderlying = record.reg_layer_priv_key_shard().second;
		shardValUnderlying.Encode(res.data() - SHARD_VALUE_MAX_SIZE, SHARD_VALUE_MAX_SIZE);

		// if owner, no more data to write
		if (!record.is_owner())
			return res;

		// else, write remaining data (owner shard)
		res.resize(res.size() + SHARD_ID_MAX_SIZE);
		record.owner_layer_priv_key_shard().first.Encode(res.data() - SHARD_ID_MAX_SIZE, SHARD_ID_MAX_SIZE);
		res.resize(res.size() + SHARD_VALUE_MAX_SIZE);
		shardValUnderlying = record.owner_layer_priv_key_shard().second;
		shardValUnderlying.Encode(res.data() - SHARD_VALUE_MAX_SIZE, SHARD_VALUE_MAX_SIZE);

		return res;
	}

	ProfileDataIterator::ProfileDataIterator(const ProfileEncKey& key,
												   ProfileInputFile& file,
												   utils::file_pos_t pos)
		: _key(key), _file(file), _pos(pos),
		  _recordEncSizes(ProfileUtils::read_profile_record_enc_sizes(_file)),
		  _record(ProfileUtils::read_profile_record(_file, _key, _recordEncSizes)) { }

	bool ProfileDataIterator::operator==(const Self& other) const
	{
		return (this->_pos == other._pos);
	}

	ProfileDataIterator::Self& ProfileDataIterator::operator++()
	{
		this->_pos = next_pos();
		this->_file.get().set_pos(this->_pos);
		this->_recordEncSizes = ProfileUtils::read_profile_record_enc_sizes(_file);
		this->_record = ProfileUtils::read_profile_record(_file, _key, _recordEncSizes);
		return *this;
	}

	ProfileDataIterator::Self ProfileDataIterator::operator++(int)
	{
		return Self(_key, _file, next_pos());
	}

	ProfileDataIterator::reference ProfileDataIterator::operator*() const
	{
		return _record;
	}

	ProfileDataIterator::pointer ProfileDataIterator::operator->() const
	{
		return &_record;
	}

	utils::file_pos_t ProfileDataIterator::next_pos() const
	{
		// next record starts after sizes and record ciphertext
		return this->_pos + 
			sizeof(std::tuple_element_t<0, profile_record_enc_sizes_t>) +
			sizeof(std::tuple_element_t<1, profile_record_enc_sizes_t>) +
			_recordEncSizes.first + _recordEncSizes.second;
	}

	ProfileDataRange::ProfileDataRange(const std::string& path, const ProfileEncKey& key)
		: _file(path), _key(key) { }

	ProfileDataRange::ProfileDataRange(Self&& other) noexcept
		: _file(std::move(other._file)), _key(std::move(other._key)) { }

	ProfileDataRange::Self& ProfileDataRange::operator=(Self other)
	{
		this->swap(other);
		return *this;
	}

	void ProfileDataRange::swap(Self& other)
	{
		this->_file.swap(other._file);
		std::swap(this->_key, other._key);
	}

	ProfileDataRange::iterator ProfileDataRange::begin()
	{
		return iterator(_key, _file);
	}

	ProfileDataRange::iterator ProfileDataRange::end()
	{
		return iterator(_key, _file, _file.size());
	}

	ProfileStorage::ProfileStorage(const std::string& path,
								   const std::string& username,
								   const std::string& password)
		: _path(path), _key(derive_key(username, password)) { }

	ProfileDataRange ProfileStorage::iter_profile_data() const
	{
		return ProfileDataRange(_path, _key);
	}

	void ProfileStorage::add_profile_data(const ProfileRecord& record)
	{
		ProfileOutputFile file(_path);
		ProfileUtils::write_profile_record_with_enc_sizes(file, _key, record);
	}

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
