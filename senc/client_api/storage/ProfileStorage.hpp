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
#include "ProfileRecord.hpp"

namespace senc::clientapi::storage
{
	/**
	 * @typedef senc::clientapi::storage::profile_record_size_t
	 * @brief Size of profile record on stored memory.
	 */
	using profile_record_size_t = std::uint64_t;

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
		utils::enc::AES1L::Key _key;

		/**
		 * @brief Derives key for profile access from username and password.
		 * @param username Username to derive key from.
		 * @param password Password to derive key from.
		 * @return Derived key.
		 */
		static utils::enc::AES1L::Key derive_key(const std::string& username, const std::string& password);
	};
}
