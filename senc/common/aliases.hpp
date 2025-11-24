/*********************************************************************
 * \file   aliases.hpp
 * \brief  Contains type aliases for both server and client.
 * 
 * \author aviad1b
 * \date   November 2025, Kislev 5786
 *********************************************************************/

#include "../utils/enc/HybridElGamal2L.hpp"
#include "../utils/enc/ECHKDF2L.hpp"
#include "../utils/enc/AES1L.hpp"
#include "../utils/ECGroup.hpp"
#include "../utils/Shamir.hpp"

namespace senc
{
	using Schema = utils::enc::HybridElGamal2L<
		utils::ECGroup, utils::enc::AES1L, utils::enc::ECHKDF2L
	>;

	using Shamir = utils::ShamirHybridElGamal<
		utils::ECGroup, utils::enc::AES1L, utils::enc::ECHKDF2L
	>;

	using PubKey = utils::enc::PubKey<Schema>;
	using PrivKey = utils::enc::PrivKey<Schema>;

	static_assert(std::same_as<utils::enc::Plaintext<Schema>, utils::Buffer>);

	using Ciphertext = utils::enc::Ciphertext<Schema>;

	using Shard = typename Shamir::Shard;
}
