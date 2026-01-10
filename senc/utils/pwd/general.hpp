/*********************************************************************
 * \file   general.hpp
 * \brief  Contains general concepts and type traits for passwords.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include <concepts>
#include <string>

namespace senc::utils::pwd
{
	/**
	 * @concept senc::utils::pwrd::PwdHasher
	 * @brief Looks for a password hash schema.
	 * @tparam Self Examined typename.
	 * @tparam Password Password type (defaults to `std::string`).
	 */
	template <typename Self, typename Password = std::string>
	concept PwdHasher = requires(Self self, const Password password, const typename Self::Salt salt)
	{
		typename Self::Salt;
		typename Self::Output;
		{ self.generate_salt() } -> std::convertible_to<typename Self::Salt>;
		{ self.hash(password, salt) } -> std::convertible_to<typename Self::Output>;
	};

	/**
	 * @typedef senc::utils::pwd::Salt
	 * @brief Type of password hasher's salt value.
	 * @tparam PH Password hasher type.
	 */
	template <PwdHasher PH>
	using Salt = typename PH::Salt;

	/**
	 * @typedef senc::utils::pwd::PwdHash
	 * @brief Type of password hasher's hash output.
	 * @tparam PH Password hasher type.
	 */
	template <PwdHasher PH>
	using PwdHash = typename PH::Output;
}
