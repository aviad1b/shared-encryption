/*********************************************************************
 * \file   Asymmetric2L.hpp
 * \brief  Contains `enc::Asymmetric2L` concept.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <tuple>
#include "../concepts.hpp"
#include "../Group.hpp"

namespace senc::utils::enc
{
	/**
	 * @concept senc::utils::enc::Asymmetric2L
	 * @brief Looks for a typename which implements an asymmetric two-layer encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Asymmetric2L = requires(const Self self,
		const typename Self::Plaintext plaintext, const typename Self::Cyphertext ciphertext,
		const typename Self::PubKey pubKey1, const typename Self::PubKey pubKey2,
		const typename Self::PrivKey privKey1, const typename Self::PrivKey privKey2)
	{
		typename Self::PrivKey;
		typename Self::PubKey;
		typename Self::Plaintext;
		typename Self::Cyphertext;
		{ self.keygen() } -> std::same_as<std::tuple<typename Self::PrivKey, typename Self::PubKey>>;
		{ self.encrypt(plaintext, pubKey1, pubKey2) } -> std::same_as<typename Self::Cyphertext>;
		{ self.decrypt(ciphertext, privKey1, privKey2) } -> std::same_as<typename Self::Plaintext>;
	};

	/**
	 * @typedef senc::utils::enc::PrivKey
	 * @brief Private key type of an asymmetric two-layer encryption schema.
	 * @tparam Schema Asymmetric two-layer encryption schema implementing typename.
	 */
	template <Asymmetric2L Schema>
	using PrivKey = typename Schema::PrivKey;

	/**
	 * @typedef senc::utils::enc::PubKey
	 * @brief Public key type of an asymmetric two-layer encryption schema.
	 * @tparam Schema Asymmetric two-layer encryption schema implementing typename.
	 */
	template <Asymmetric2L Schema>
	using PubKey = typename Schema::PubKey;

	/**
	 * @typedef senc::utils::enc::Plaintext
	 * @brief Plaintext type of an asymmetric two-layer encryption schema.
	 * @tparam Schema Asymmetric two-layer encryption schema implementing typename.
	 */
	template <Asymmetric2L Schema>
	using Plaintext = typename Schema::Plaintext;

	/**
	 * @typedef senc::utils::enc::Ciphertext
	 * @brief Ciphertext type of an asymmetric two-layer encryption schema.
	 * @tparam Schema Asymmetric two-layer encryption schema implementing typename.
	 */
	template <Asymmetric2L Schema>
	using Ciphertext = typename Schema::Ciphertext;
}
