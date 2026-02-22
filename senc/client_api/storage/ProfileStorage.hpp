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
	 * @typedef senc::clientapi::storage::profile_record_enc_sizes_t
	 * @brief Sizes of profile record encryption on stored memory.
	 */
	using profile_record_enc_sizes_t = std::pair<std::uint16_t, std::uint16_t>;

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
	using ProfileOutputFile = utils::BinFile<utils::AccessFlags::Append>;
	using ProfileEncSchema = utils::enc::AES1L;
	static_assert(std::same_as<utils::enc::Plaintext<ProfileEncSchema>, utils::Buffer>);
	using ProfileEncKey = utils::enc::Key<ProfileEncSchema>;
	using ProfileEncCiphertext = utils::enc::Ciphertext<ProfileEncSchema>;
	static_assert(2 == std::tuple_size_v<ProfileEncCiphertext>, "Ciphertext type changed, requires refactor");

	/**
	 * @class senc::clientapi::storage::ProfileUtils
	 * @brief Contains utility methods for profile storage.
	 */
	class ProfileUtils
	{
	public:
		static profile_record_enc_sizes_t read_profile_record_enc_sizes(ProfileInputFile& file);

		static ProfileRecord read_profile_record(ProfileInputFile& file,
												 const ProfileEncKey& key,
												 profile_record_enc_sizes_t sizes);

		static void write_profile_record_with_enc_sizes(ProfileOutputFile& file,
														const ProfileEncKey& key,
														const ProfileRecord& record);

	private:
		// this constant is true as shard IDs are currently sampled from [1,MAX_MEMBER_COUNT]
		// TODO: move this constant elsewhere in future version
		static constexpr std::size_t SHARD_ID_MAX_SIZE = sizeof(member_count_t);

		// this constant is true as shard values are always < modulus
		// TODO: move this constant elsewhere in future version
		static const std::size_t SHARD_VALUE_MAX_SIZE;

		static ProfileEncSchema& schema();

		static ProfileRecord parse_profile_record(utils::Buffer& data);
		// TODO: Add const once utils::read_bytes accepts const_iterator

		static utils::Buffer serialize_profile_record(const ProfileRecord& record);
	};

	/**
	 * @class senc::clientapi::storage::ProfileStorageIterator
	 * @brief Used for iteration over profile storage.
	 */
	class ProfileDataIterator
	{
	public:
		using Self = ProfileDataIterator;
		using value_type = const ProfileRecord;
		using reference = const ProfileRecord&;
		using pointer = const ProfileRecord*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::input_iterator_tag;
		using iterator_concept = std::input_iterator_tag;

		ProfileDataIterator(const ProfileEncKey& key,
							ProfileInputFile& file,
							utils::file_pos_t pos = 0);

		ProfileDataIterator(const Self&) = default;

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
		profile_record_enc_sizes_t _recordEncSizes;
		ProfileRecord _record;

		utils::file_pos_t next_pos() const;
	};

	/**
	 * @class senc::clientapi::storage::ProfileDataRange
	 * @brief Used as a range iterating over a profile's data.
	 */
	class ProfileDataRange
	{
	public:
		using Self = ProfileDataRange;
		using iterator = ProfileDataIterator;

		ProfileDataRange(const std::string& path, const ProfileEncKey& key);

		ProfileDataRange(const Self&) = delete;

		Self operator=(const Self&) = delete;

		ProfileDataRange(Self&& other) noexcept;

		Self& operator=(Self other);

		void swap(Self& other);

		iterator begin();

		iterator end();

	private:
		ProfileInputFile _file;
		std::reference_wrapper<const ProfileEncKey> _key;
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
		 * @param path Profile file path.
		 * @param username User's username.
		 * @param password User's password.
		 * @note This class does not store the password, only uses it to derive key for profile access.
		 */
		ProfileStorage(const std::string& path, const std::string& username, const std::string& password);

		/**
		 * @brief Gets a range iterating over profile's data.
		 */
		ProfileDataRange iter_profile_data() const;

		/**
		 * @brief Adds profile record to profile storage.
		 */
		void add_profile_data(const ProfileRecord& record);

	private:
		std::string _path;
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
