/*********************************************************************
 * \file   ProfileStorage.hpp
 * \brief  Header of ProfileStorage class and utilities.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#pragma once

#include "../../utils/pwd/PBKDF2.hpp"
#include "../../utils/enc/AES1L.hpp"
#include "../../utils/BinFile.hpp"
#include "../../common/sizes.hpp"
#include "ProfileRecord.hpp"

namespace senc::clientapi::storage
{
	/**
	 * @typedef senc::clientapi::storage::profile_record_enc_size_t
	 * @brief Size of profile record encryption part on stored memory.
	 */
	using profile_record_enc_size_t = std::uint16_t;

	/**
	 * @struct senc::clientapi::storage::ProfileRecordFlags
	 * @brief Flags stored for each profile record on memory.
	 */
	struct ProfileRecordFlags
	{
		bool is_owner : 1;

		static ProfileRecordFlags from_byte(utils::byte b)
		{
			return {
				static_cast<bool>(b & (1 << 0))
			};
		}

		utils::byte to_byte() const
		{
			return (static_cast<std::uint8_t>(is_owner) << 0);
		}
	};

	using ProfileInputFile = utils::BinFile<utils::AccessFlags::Read>;
	using ProfileEncSchema = utils::enc::AES1L;
	static_assert(std::same_as<utils::enc::Plaintext<ProfileEncSchema>, utils::Buffer>);
	using ProfileEncKey = utils::enc::Key<ProfileEncSchema>;
	using ProfileEncCiphertext = utils::enc::Ciphertext<ProfileEncSchema>;
	static_assert(2 == std::tuple_size_v<ProfileEncCiphertext>, "Ciphertext type changed, requires refactor");

	/**
	 * @class senc::clientapi::storage::ProfileStorageIterator
	 * @brief Used for iteration over profile storage.
	 */
	class ProfileStorageIterator
	{
	public:
		using Self = ProfileStorageIterator;
		using value_type = const ProfileRecord;
		using reference = const ProfileRecord&;
		using pointer = const ProfileRecord*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::input_iterator_tag;
		using iterator_concept = std::input_iterator_tag;

		ProfileStorageIterator(const ProfileEncKey& key,
							   ProfileInputFile& file,
							   utils::file_pos_t pos = 0);

		ProfileStorageIterator(const Self&) = default;

		Self& operator=(const Self&) = default;

		bool operator==(const Self& other) const;

		Self& operator++();

		Self operator++(int);

		reference operator*() const;

		pointer operator->() const;

	private:
		std::reference_wrapper<const ProfileEncKey> _key;
		std::reference_wrapper<ProfileInputFile> _file;
		utils::file_pos_t _pos;
		profile_record_enc_size_t _recordEncSize1, _recordEncSize2;
		ProfileRecord _record;

		utils::file_pos_t next_pos() const;

		static profile_record_enc_size_t read_profile_record_enc_size(ProfileInputFile& file);

		static ProfileRecord read_profile_record(ProfileInputFile& file,
												 const ProfileEncKey& key,
												 profile_record_enc_size_t size1,
												 profile_record_enc_size_t size2);

		static ProfileRecord parse_profile_record(const ProfileEncKey& key, utils::Buffer& data);
		// TODO: Add const once utils::read_bytes accepts const_iterator

		static ProfileEncSchema& schema();
	};

	/**
	 * @class senc::clientapi::storage::ProfileDataRange
	 * @brief Used as a range iterating over a profile's data.
	 */
	class ProfileDataRange
	{
	public:
		using File = utils::BinFile<utils::AccessFlags::Read>;

	private:
		std::reference_wrapper<utils::enc::AES1L::Key> _key;
		File _file;
	};

	/**
	 * @class senc::clientapi::storage::ProfileStorage
	 * @brief Manages storage of client's profile.
	 */
	class ProfileStorage
	{
	public:
		using Self = ProfileStorage;

		/**
		 * @brief Constructs a profile storage handling instance.
		 * @param username User's username.
		 * @param password User's password.
		 * @note This class does not store the password, only uses it to derive key for profile access.
		 */
		ProfileStorage(const std::string& username, const std::string& password);

	private:
		ProfileEncKey _key;

		/**
		 * @brief Derives key for profile access from username and password.
		 * @param username Username to derive key from.
		 * @param password Password to derive key from.
		 * @return Derived key.
		 */
		static ProfileEncKey derive_key(const std::string& username, const std::string& password);
	};
}
