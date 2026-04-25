/*********************************************************************
 * \file   ProfileStorage.cpp
 * \brief  Implementation of ProfileStorage class and utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "ProfileStorage.hpp"

#include "../ClientException.hpp"
#include "../../utils/swap.hpp"

namespace senc::clientapi::storage
{
	profile_record_enc_sizes_t ProfileUtils::read_profile_record_enc_sizes(ProfileInputFile& file)
	{
		profile_record_enc_sizes_t res{};
		if (file.get_pos() >= file.size())
			return res; // nothing to read, file ended
		file.read(res);
		return res;
	}

	std::optional<ProfileRecord> ProfileUtils::read_profile_record(ProfileInputFile& file,
																   const ProfileEncKey& key,
																   profile_record_enc_sizes_t sizes)
	{
		// if sizes are all zero
		if (std::apply([](auto&&... args) { return ((0 == args) && ...); }, sizes))
			return std::nullopt; // nothing to read

		// read encrypted profile
		ProfileEncCiphertext encProfile{};
		[&encProfile, &sizes, &file]<std::size_t... is>(std::index_sequence<is...>) -> void
		{
			([&sizes, &encProfile, &file]()
			{
				auto size = std::get<is>(sizes);
				auto& buff = std::get<is>(encProfile);
				buff.resize(size);
				file.read(buff.data(), size);
			}(), ...);
		}(std::make_index_sequence<std::tuple_size_v<ProfileEncCiphertext>>{});

		// decrypt
		utils::Buffer profileBytes = schema().decrypt(encProfile, key);

		// parse
		return parse_profile_record(profileBytes);
	}

	ProfileEncCiphertext ProfileUtils::encrypt_profile_record(const ProfileRecord& record,
															  const ProfileEncKey& key)
	{
		auto profileBytes = serialize_profile_record(record);
		return schema().encrypt(profileBytes, key);
	}

	void ProfileUtils::write_profile_record_with_enc_sizes(ProfileOutputFile& file,
														   const ProfileEncCiphertext& enc)
	{
		auto sizes = [&enc]<std::size_t... is>(std::index_sequence<is...>) -> profile_record_enc_sizes_t
		{
			return profile_record_enc_sizes_t(
				static_cast<std::tuple_element_t<is, profile_record_enc_sizes_t>>(
					std::get<is>(enc).size()
				)...
			);
		}(std::make_index_sequence<std::tuple_size_v<profile_record_enc_sizes_t>>{});

		// append sizes to file
		file.append(sizes);

		// append enc parts to file
		std::apply([&file](const auto&... parts)
		{
			(file.append(parts.data(), parts.size()), ...);
		}, enc);
	}

	void ProfileUtils::write_profile_record_with_enc_sizes(ProfileInputFile& file, utils::file_pos_t pos, const ProfileEncCiphertext& enc)
	{
		auto sizes = [&enc]<std::size_t... is>(std::index_sequence<is...>) -> profile_record_enc_sizes_t
		{
			return profile_record_enc_sizes_t(
				static_cast<std::tuple_element_t<is, profile_record_enc_sizes_t>>(
					std::get<is>(enc).size()
					)...
			);
		}(std::make_index_sequence<std::tuple_size_v<profile_record_enc_sizes_t>>{});

		file.set_pos(pos);

		// write sizes to file
		file.write(sizes);

		// append enc parts to file
		std::apply([&file](const auto&... parts)
		{
			(file.write(parts.data(), parts.size()), ...);
		}, enc);
	}

	void ProfileUtils::write_profile_record_with_enc_sizes(ProfileOutputFile& file,
														   const ProfileEncKey& key,
														   const ProfileRecord& record)
	{
		write_profile_record_with_enc_sizes(
			file,
			encrypt_profile_record(record, key)
		);
	}

	void ProfileUtils::write_profile_record_with_enc_sizes(ProfileInputFile& file, utils::file_pos_t pos, const ProfileEncKey& key, const ProfileRecord& record)
	{
		write_profile_record_with_enc_sizes(
			file, pos,
			encrypt_profile_record(record, key)
		);
	}

	ProfileEncSchema& ProfileUtils::schema()
	{
		static thread_local ProfileEncSchema schema;
		return schema;
	}

	ProfileRecord ProfileUtils::parse_profile_record(const utils::Buffer& data)
	{
		utils::BytesView view(data.data(), data.size());
		const auto end = view.end();
		auto it = view.begin();

		utils::byte flagsByte = 0;
		it = utils::read_bytes(flagsByte, it, end);
		const auto flags = ProfileRecordFlags::from_byte(flagsByte);

		UserSetID usersetID{};
		it = utils::read_bytes(usersetID, it, end);

		utils::BigInt nextEvolutionOffset{};
		it = read_evolution_offset(nextEvolutionOffset, it, end);

		PubKey regLayerPubKey{}, ownerLayerPubKey{};
		it = read_pub_key(regLayerPubKey, it, end);
		it = read_pub_key(ownerLayerPubKey, it, end);

		PrivKeyShard regExternalPrivKeyShard{};
		it = read_priv_key_shard(regExternalPrivKeyShard, it, end);

		// if non-owner record, stop here - no more data to read
		if (!flags.is_owner)
			return ProfileRecord::reg(
				std::move(usersetID),
				std::move(nextEvolutionOffset),
				std::move(regLayerPubKey), std::move(ownerLayerPubKey),
				std::move(regExternalPrivKeyShard)
			);

		// otherwise, read remaining data (owner shards) then return
		PrivKeyShard regInternalPrivKeyShard, ownerExternalPrivKeyShard{}, ownerInternalPrivKeyShard{};
		it = read_priv_key_shard(regInternalPrivKeyShard, it, end);
		it = read_priv_key_shard(ownerExternalPrivKeyShard, it, end);
		it = read_priv_key_shard(ownerInternalPrivKeyShard, it, end);

		return ProfileRecord::owner(
			std::move(usersetID),
			std::move(nextEvolutionOffset),
			std::move(regLayerPubKey), std::move(ownerLayerPubKey),
			std::move(regExternalPrivKeyShard), std::move(regInternalPrivKeyShard),
			std::move(ownerExternalPrivKeyShard), std::move(ownerInternalPrivKeyShard)
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

		utils::write_bytes(res, record.userset_id());

		write_evolution_offset(res, record.next_evolution_offset());

		write_pub_key(res, record.reg_pub_key());
		write_pub_key(res, record.owner_pub_key());

		write_priv_key_shard(res, record.reg_external_priv_key_shard());

		// if owner, no more data to write
		if (!record.is_owner())
			return res;

		// else, write remaining data (owner shards)
		write_priv_key_shard(res, record.reg_internal_priv_key_shard());
		write_priv_key_shard(res, record.owner_external_priv_key_shard());
		write_priv_key_shard(res, record.owner_internal_priv_key_shard());

		return res;
	}

	ProfileHolder::ProfileHolder(const ProfileEncKey& key,
								 ProfileInputFile& file,
								 std::mutex& mtxFile,
								 utils::file_pos_t pos,
								 profile_record_enc_sizes_t& recordEncSizes,
								 ProfileRecord& record)
		: _key(key), _file(file), _mtxFile(mtxFile), _pos(pos),
		  _recordEncSizes(recordEncSizes), _record(record) { }

	ProfileHolder::operator const ProfileRecord&() const noexcept
	{
		return _record;
	}

	ProfileRecord* ProfileHolder::operator&() noexcept
	{
		return &_record;
	}

	const ProfileRecord* ProfileHolder::operator&() const noexcept
	{
		return &_record;
	}

	ProfileHolder::Self& ProfileHolder::operator=(ProfileRecord record)
	{
		// encrypt new record and check for same size
		// (for now, changing size is not supported)
		auto sum = [](auto&&... args) { return (args + ...); };
		auto sizeSum = [](auto&&... args) { return (args.size() + ...); };
		const auto enc = ProfileUtils::encrypt_profile_record(record, _key);
		const auto oldTotalSize = static_cast<std::size_t>(std::apply(sum, _recordEncSizes));
		const auto newTotalSize = static_cast<std::size_t>(std::apply(sizeSum, enc));
		if (newTotalSize != oldTotalSize)
			throw ClientException("Client storage attempted to change size - not supported yet");

		// re-write profile to file and assign locally
		this->_record = std::move(record);
		const std::lock_guard lock(_mtxFile);
		ProfileUtils::write_profile_record_with_enc_sizes(_file, _pos, enc);

		return *this;
	}

	ProfileDataIterator::ProfileDataIterator(const ProfileEncKey& key,
		ProfileInputFile& file,
		std::mutex& mtxFile,
		utils::file_pos_t pos)
		: _key(key), _file(file), _mtxFile(mtxFile), _pos(pos)
	{
		const std::lock_guard lock(_mtxFile.get());
		_recordEncSizes = ProfileUtils::read_profile_record_enc_sizes(_file);
		_record = ProfileUtils::read_profile_record(_file, _key, _recordEncSizes);
	}

	bool ProfileDataIterator::operator==(const Self& other) const
	{
		return (this->_pos == other._pos);
	}

	ProfileDataIterator::Self& ProfileDataIterator::operator++()
	{
		const std::lock_guard lock(_mtxFile.get());
		this->_pos = next_pos();
		this->_file.get().set_pos(this->_pos);
		this->_recordEncSizes = ProfileUtils::read_profile_record_enc_sizes(_file);
		this->_record = ProfileUtils::read_profile_record(_file, _key, _recordEncSizes);

		return *this;
	}

	ProfileDataIterator::Self ProfileDataIterator::operator++(int)
	{
		return Self(_key, _file, _mtxFile, next_pos());
	}

	ProfileDataIterator::reference ProfileDataIterator::operator*()
	{
		return { _key, _file, _mtxFile, _pos, _recordEncSizes, *_record };
	}

	ProfileDataIterator::pointer ProfileDataIterator::operator->()
	{
		return &(**this);
	}

	utils::file_pos_t ProfileDataIterator::next_pos() const
	{
		// next record starts after sizes and record ciphertext
		return this->_pos +
			sizeof(std::tuple_element_t<0, profile_record_enc_sizes_t>) +
			sizeof(std::tuple_element_t<1, profile_record_enc_sizes_t>) +
			std::apply([](auto&&... args) { return (args + ...); }, _recordEncSizes);
	}

	ProfileDataRange::ProfileDataRange(const std::string& path, const ProfileEncKey& key, std::mutex& mtxFile)
		: _file(path), _key(key), _mtxFile(mtxFile) { }

	ProfileDataRange::ProfileDataRange(Self&& other) noexcept
		: _file(std::move(other._file)),
		  _key(std::move(other._key)),
		  _mtxFile(std::move(other._mtxFile)) { }

	ProfileDataRange::Self& ProfileDataRange::operator=(Self other)
	{
		utils::swap(*this, other);
		return *this;
	}

	void ProfileDataRange::swap(Self& other)
	{
		utils::swap(this->_file, other._file);
		utils::swap(this->_key, other._key);
	}

	ProfileDataRange::iterator ProfileDataRange::begin()
	{
		return iterator(_key, _file, _mtxFile);
	}

	ProfileDataRange::iterator ProfileDataRange::end()
	{
		return iterator(_key, _file, _mtxFile, _file.size());
	}

	ProfileStorage::ProfileStorage(const std::string& path,
								   const std::string& username,
								   const std::string& password)
		: Self(std::string(path), username, password) { }

	ProfileStorage::ProfileStorage(std::string&& path,
								   const std::string& username,
								   const std::string& password)
		: _username(username), _path(std::move(path)), _key(derive_key(username, password))
	{
		// create file if doesn't exist
		utils::BinFile<utils::AccessFlags::Read | utils::AccessFlags::Append> f(_path);
	}

	const std::string& ProfileStorage::username() const
	{
		return this->_username;
	}

	ProfileDataRange ProfileStorage::iter_profile_data()
	{
		const std::lock_guard lock(_mtxFile);
		return ProfileDataRange(_path, _key, _mtxFile);
	}

	void ProfileStorage::add_profile_data(const ProfileRecord& record)
	{
		const std::lock_guard lock(_mtxFile);
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
