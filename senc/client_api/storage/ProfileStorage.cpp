/*********************************************************************
 * \file   ProfileStorage.cpp
 * \brief  Implementation of ProfileStorage class and utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "ProfileStorage.hpp"

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

	void ProfileUtils::write_profile_record_with_enc_sizes(ProfileOutputFile& file,
														   const ProfileEncKey& key,
														   const ProfileRecord& record)
	{
		auto profileBytes = serialize_profile_record(record);
		const auto enc = schema().encrypt(profileBytes, key);
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

		PubKey regLayerPubKey{}, ownerLayerPubKey{};
		it = read_pub_key(regLayerPubKey, it, end);
		it = read_pub_key(ownerLayerPubKey, it, end);

		PrivKeyShard regLayerPrivKeyShard{};
		it = read_priv_key_shard(regLayerPrivKeyShard, it, end);

		// if non-owner record, stop here - no more data to read
		if (!flags.is_owner)
			return ProfileRecord::reg(
				std::move(usersetID),
				std::move(regLayerPubKey), std::move(ownerLayerPubKey),
				std::move(regLayerPrivKeyShard)
			);

		// otherwise, read remaining data (owner shard) then return
		PrivKeyShard ownerLayerPrivKeyShard{};
		it = read_priv_key_shard(ownerLayerPrivKeyShard, it, end);

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

		utils::write_bytes(res, record.userset_id());

		write_pub_key(res, record.reg_layer_pub_key());
		write_pub_key(res, record.owner_layer_pub_key());

		write_priv_key_shard(res, record.reg_layer_priv_key_shard());

		// if owner, no more data to write
		if (!record.is_owner())
			return res;

		// else, write remaining data (owner shard)
		write_priv_key_shard(res, record.owner_layer_priv_key_shard());

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
		return *_record;
	}

	ProfileDataIterator::pointer ProfileDataIterator::operator->() const
	{
		return std::to_address(_record);
	}

	utils::file_pos_t ProfileDataIterator::next_pos() const
	{
		// next record starts after sizes and record ciphertext
		return this->_pos +
			sizeof(std::tuple_element_t<0, profile_record_enc_sizes_t>) +
			sizeof(std::tuple_element_t<1, profile_record_enc_sizes_t>) +
			std::apply([](auto&&... args) { return (args + ...); }, _recordEncSizes);
	}

	ProfileDataRange::ProfileDataRange(const std::string& path, const ProfileEncKey& key)
		: _file(path), _key(key) { }

	ProfileDataRange::ProfileDataRange(Self&& other) noexcept
		: _file(std::move(other._file)), _key(std::move(other._key)) { }

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
		return iterator(_key, _file);
	}

	ProfileDataRange::iterator ProfileDataRange::end()
	{
		return iterator(_key, _file, _file.size());
	}

	ProfileStorage::ProfileStorage(const std::string& path,
								   const std::string& username,
								   const std::string& password)
		: Self(std::string(path), username, password) { }

	ProfileStorage::ProfileStorage(std::string&& path,
								   const std::string& username,
								   const std::string& password)
		: _path(std::move(path)), _key(derive_key(username, password)) { }

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
