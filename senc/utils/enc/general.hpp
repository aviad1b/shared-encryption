/*********************************************************************
 * \file   general.hpp
 * \brief  Contains general concepts and type traits for encryption.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <concepts>
#include <tuple>

namespace senc::utils::enc
{
	/**
	 * @concept senc::utils::enc::Schema
	 * @brief Looks for an encryption schema typename.
	 * @tparam Self Examiend typename.
	 */
	template <typename Self>
	concept Schema = requires
	{
		typename Self::Plaintext;
		typename Self::Ciphertext;
	};

	/**
	 * @typedef senc::utils::enc::Plaintext
	 * @brief Plaintext type of a encryption schema.
	 * @tparam S Symmetric one-layer encryption schema implementing typename.
	 */
	template <Schema S>
	using Plaintext = typename S::Plaintext;

	/**
	 * @typedef senc::utils::enc::Ciphertext
	 * @brief Ciphertext type of a encryption schema.
	 * @tparam S Symmetric one-layer encryption schema implementing typename.
	 */
	template <Schema S>
	using Ciphertext = typename S::Ciphertext;

	/**
	 * @concept senc::utils::enc::Symmetric
	 * @brief Looks for a typename which implements any symmetric encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Symmetric = Schema<Self> && requires(const Self self)
	{
		typename Self::Key;
		{ self.keygen() } -> std::same_as<typename Self::Key>;
	};

	/**
	 * @typedef senc::utils::enc::Key
	 * @brief Key type of a symmetric encryption schema.
	 * @tparam S Symmetric encryption schema implementing typename.
	 */
	template <Symmetric S>
	using Key = typename S::Key;

	/**
	 * @concept senc::utils::enc::Asymmetric
	 * @brief Looks for a typename which implements any asymmetric encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Asymmetric = Schema<Self> && requires(const Self self)
	{
		typename Self::PrivKey;
		typename Self::PubKey;
		{ self.keygen() } -> std::same_as<std::tuple<typename Self::PrivKey, typename Self::PubKey>>;
	};

	/**
	 * @typedef senc::utils::enc::PrivKey
	 * @brief Private key type of an asymmetric encryption schema.
	 * @tparam S Asymmetric encryption schema implementing typename.
	 */
	template <Asymmetric S>
	using PrivKey = typename S::PrivKey;

	/**
	 * @typedef senc::utils::enc::PubKey
	 * @brief Public key type of an asymmetric encryption schema.
	 * @tparam S Asymmetric encryption schema implementing typename.
	 */
	template <Asymmetric S>
	using PubKey = typename S::PubKey;

	/**
	 * @concept senc::utils::enc::Symmetric1L
	 * @brief Looks for a typename which implements a symmetric one-layer encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Symmetric1L = Symmetric<Self> && requires(
		const Self self, const Key<Self> key,
		const Plaintext<Self> plaintext,
		const Ciphertext<Self> ciphertext
	)
	{
		{ self.encrypt(plaintext, key) } -> std::same_as<Ciphertext<Self>>;
		{ self.decrypt(ciphertext, key) } -> std::same_as<Plaintext<Self>>;
	};

	/**
	 * @concept senc::utils::enc::Symmetric2L
	 * @brief Looks for a typename which implements a symmetric two-layer encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Symmetric2L = Symmetric<Self> && requires(
		const Self self, const Key<Self> key1, const Key<Self> key2,
		const Plaintext<Self> plaintext, const Ciphertext<Self> ciphertext
	)
	{
		{ self.encrypt(plaintext, key1, key2) } -> std::same_as<Ciphertext<Self>>;
		{ self.decrypt(ciphertext, key1, key2) } -> std::same_as<Plaintext<Self>>;
	};

	/**
	 * @concept senc::utils::enc::Asymmetric1L
	 * @brief Looks for a typename which implements an asymmetric one-layer encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Asymmetric1L = Asymmetric<Self> && requires(
		const Self self,
		const PubKey<Self> pubKey, const PrivKey<Self> privKey,
		const Plaintext<Self> plaintext, const Ciphertext<Self> ciphertext
	)
	{
		{ self.encrypt(plaintext, pubKey) } -> std::same_as<Ciphertext<Self>>;
		{ self.decrypt(ciphertext, privKey) } -> std::same_as<Plaintext<Self>>;
	};

	/**
	 * @concept senc::utils::enc::Asymmetric2L
	 * @brief Looks for a typename which implements an asymmetric two-layer encryption schema.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Asymmetric2L = Asymmetric<Self> && requires(
		const Self self,
		const PubKey<Self> pubKey1, const PubKey<Self> pubKey2,
		const PrivKey<Self> privKey1, const PrivKey<Self> privKey2,
		const Plaintext<Self> plaintext, const Ciphertext<Self> ciphertext
	)
	{
		{ self.encrypt(plaintext, pubKey1, pubKey2) } -> std::same_as<Ciphertext<Self>>;
		{ self.decrypt(ciphertext, privKey1, privKey2) } -> std::same_as<Plaintext<Self>>;
	};
}
