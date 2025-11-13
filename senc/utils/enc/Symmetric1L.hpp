/*********************************************************************
 * \file   Symmetric1L.hpp
 * \brief  Contains `enc::Symmetric1L` concept.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <concepts>

namespace senc::utils::enc
{
	/**
	 * @concept senc::utils::enc::Symmetric1L
	 * @brief Looks for a typename which implements a symmetric one-layer encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Symmetric1L = requires(const Self self,
		const typename Self::Plaintext plaintext,
		const typename Self::Ciphertext ciphertext,
		const typename Self::Key key)
	{
		typename Self::Key;
		typename Self::Plaintext;
		typename Self::Ciphertext;
		{ self.keygen() } -> std::same_as<typename Self::Key>;
		{ self.encrypt(plaintext, key) } -> std::same_as<typename Self::Ciphertext>;
		{ self.decrypt(ciphertext, key) } -> std::same_as<typename Self::Plaintext>;
	};

	/**
	 * @typedef senc::utils::enc::Key
	 * @brief Key type of a symmetric one-layer encryption schema.
	 * @tparam Schema Symmetric one-layer encryption schema implementing typename.
	 */
	template <Symmetric1L Schema>
	using Key = typename Schema::Key;

	/**
	 * @typedef senc::utils::enc::Plaintext
	 * @brief Plaintext type of a symmetric one-layer encryption schema.
	 * @tparam Schema Symmetric one-layer encryption schema implementing typename.
	 */
	template <Symmetric1L Schema>
	using Plaintext = typename Schema::Plaintext;

	/**
	 * @typedef senc::utils::enc::Ciphertext
	 * @brief Ciphertext type of a symmetric one-layer encryption schema.
	 * @tparam Schema Symmetric one-layer encryption schema implementing typename.
	 */
	template <Symmetric1L Schema>
	using Ciphertext = typename Schema::Ciphertext;
}
