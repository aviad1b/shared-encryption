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

	/**
	 * @typedef senc::clientapi::storage::ProfileInputFile
	 * @brief File used for profile input.
	 */
	using ProfileInputFile = utils::BinFile<utils::AccessFlags::Read>;

	/**
	 * @typedef senc::clientapi::storage::ProfileOutputFile
	 * @brief File used for profile output.
	 */
	using ProfileOutputFile = utils::BinFile<utils::AccessFlags::Append>;

	/**
	 * @typedef senc::clientapi::storage::ProfileEncSchema
	 * @brief Encryption schema used for encrypting/decrypting profile data.
	 */
	using ProfileEncSchema = utils::enc::AES1L;
	static_assert(
		std::same_as<utils::enc::Plaintext<ProfileEncSchema>, utils::Buffer>,
		"Plaintext of profile encryption schema must be Buffer"
	);

	/**
	 * @typedef senc::clientapi::storage::ProfileEncKey
	 * @brief Key used for encrypting/decrypting profile data.
	 */
	using ProfileEncKey = utils::enc::Key<ProfileEncSchema>;

	/**
	 * @typedef senc::clientapi::storage::ProfileEncCiphertext
	 * @brief Encrypted version of profile data (ciphertext).
	 */
	using ProfileEncCiphertext = utils::enc::Ciphertext<ProfileEncSchema>;
	static_assert(
		utils::TupleSatisfies<ProfileEncCiphertext, utils::sfinae::has_byte_data> &&
		utils::TupleSatisfies<ProfileEncCiphertext, utils::sfinae::has_mutable_byte_data>,
		"Profile ciphertext must be bytes compatible"
	);
	static_assert(
		std::tuple_size_v<profile_record_enc_sizes_t> == std::tuple_size_v<ProfileEncCiphertext>,
		"profile_record_enc_sizes_t must be able to hold sizes of ProfileEncCiphertext elements"
	);

	/**
	 * @class senc::clientapi::storage::ProfileUtils
	 * @brief Contains utility methods for profile storage.
	 */
	class ProfileUtils
	{
	public:
		/**
		 * @brief Reads profile record's encryption sizes.
		 */
		static profile_record_enc_sizes_t read_profile_record_enc_sizes(ProfileInputFile& file);

		/**
		 * @brief Reads profile record (given encryption sizes and key).
		 */
		static std::optional<ProfileRecord> read_profile_record(ProfileInputFile& file,
																const ProfileEncKey& key,
																profile_record_enc_sizes_t sizes);

		/**
		 * @brief Writes (encrypted) profile record (including encryption sizes).
		 */
		static void write_profile_record_with_enc_sizes(ProfileOutputFile& file,
														const ProfileEncKey& key,
														const ProfileRecord& record);

	private:
		/**
		 * @brief Gets profile encryption schema.
		 */
		static ProfileEncSchema& schema();

		/**
		 * @brief Parses profile record from binary data.
		 * @param data Binary data.
		 * @return Parsed profile record.
		 */
		static ProfileRecord parse_profile_record(const utils::Buffer& data);

		/**
		 * @brief Serializes profile record into binary data.
		 * @param record Profile record.
		 * @return Serialized version of `record`.
		 */
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

		/**
		 * @brief Constructs a profile data iterator.
		 * @param key Reference to key used for decrypting read data.
		 * @param file Reference to file from which data is read.
		 * @param pos Reading position in file, defaults to `0` (file start).
		 */
		ProfileDataIterator(const ProfileEncKey& key,
							ProfileInputFile& file,
							utils::file_pos_t pos = 0);

		/**
		 * @brief Copy constructor of profile data iterator.
		 */
		ProfileDataIterator(const Self&) = default;

		/**
		 * @brief Copy assignment operator of profile data iterator.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Compares two profile data iterators.
		 * @note Assumes same input file.
		 * @param other Other profile data iterator to compare to.
		 * @return `true` if `*this` points to the same position as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const;

		/**
		 * @brief Increments iterator to next profile data record.
		 * @return `*this`, after incrementing.
		 */
		Self& operator++();

		/**
		 * @brief Increments iterator to next profile data record.
		 * @return `*this`, before incrementing.
		 */
		Self operator++(int);

		/**
		 * @brief Gets read profile data record.
		 * @return Read data record.
		 */
		reference operator*() const;

		/**
		 * @brief Gets read profile data record.
		 * @return Read data record.
		 */
		pointer operator->() const;

	private:
		std::reference_wrapper<const ProfileEncKey> _key;
		std::reference_wrapper<ProfileInputFile> _file;
		utils::file_pos_t _pos;
		profile_record_enc_sizes_t _recordEncSizes;
		std::optional<ProfileRecord> _record;

		/**
		 * @brief Gets start position of next profile record data in file.
		 * @return Start position of next profile record data in file.
		 */
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

		/**
		 * @brief Constructs a profile data range.
		 * @param path Path of file from which profile data is read.
		 * @param key Reference to key used for decrypting read data.
		 */
		ProfileDataRange(const std::string& path, const ProfileEncKey& key);

		ProfileDataRange(const Self&) = delete;
		Self& operator=(const Self&) = delete;

		/**
		 * @brief Move constructor of profile data range.
		 * @param other Other profile data range to construct from (moved).
		 */
		ProfileDataRange(Self&& other) noexcept;

		/**
		 * @brief Move assignment of profile data range.
		 * @param other Other profile data range to assign into this one.
		 * @note Uses inline construction (move-and-swap).
		 */
		Self& operator=(Self other);

		/**
		 * @brief Swaps with another profile data range.
		 * @param other Other profile data range to swap with.
		 */
		void swap(Self& other);

		/**
		 * @brief Gets iterator to start of profile data.
		 * @return Iterator to start of profile data.
		 */
		iterator begin();

		/**
		 * @brief Gets iterator to end of profile data.
		 * @return Iterator to end of profile data.
		 */
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
		 * @brief Constructs a profile storage handling instance.
		 * @param path Profile file path (moved).
		 * @param username User's username.
		 * @param password User's password.
		 * @note This class does not store the password, only uses it to derive key for profile access.
		 */
		ProfileStorage(std::string&& path, const std::string& username, const std::string& password);

		/**
		 * @brief Gets profile's username.
		 * @return Profile's username.
		 */
		const std::string& username() const;

		/**
		 * @brief Gets a range iterating over profile's data.
		 */
		ProfileDataRange iter_profile_data() const;

		/**
		 * @brief Adds profile record to profile storage.
		 */
		void add_profile_data(const ProfileRecord& record);

	private:
		std::string _username;
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
