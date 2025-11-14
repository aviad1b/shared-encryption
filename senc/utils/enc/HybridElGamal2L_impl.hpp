/*********************************************************************
 * \file   HybridElGamal2L_impl.hpp
 * \brief  Implementation of `enc::HybridElGamal2L` class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "HybridElGamal2L.hpp"

#include <utility>
#include "../math.hpp"

namespace senc::utils::enc
{
	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	const Distribution<CryptoPP::Integer> HybridElGamal2L<G, S, KDF>::UNDER_ORDER_DIST =
		Random<CryptoPP::Integer>::get_dist_below(G::ORDER);

	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	inline HybridElGamal2L<G, S, KDF>::HybridElGamal2L(S&& symmetricSchema, KDF&& kdf)
		: _symmetricSchema(std::forward<S>(symmetricSchema)), _kdf(std::forward<KDF>(kdf)) { }
	
	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	inline std::pair<HybridElGamal2L<G, S, KDF>::PubKey, HybridElGamal2L<G, S, KDF>::PrivKey>
		HybridElGamal2L<G, S, KDF>::keygen() const
	{
		PrivKey privKey = UNDER_ORDER_DIST();
		PubKey pubKey = senc::utils::pow(G::GENERATOR, privKey);
		return { pubKey, privKey };
	}

	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	inline HybridElGamal2L<G, S, KDF>::Ciphertext HybridElGamal2L<G, S, KDF>::encrypt(const Plaintext& plaintext, const PubKey& pubKey1, const PubKey& pubKey2) const
	{
		auto r1 = UNDER_ORDER_DIST();
		auto r2 = UNDER_ORDER_DIST();

		auto c1 = senc::utils::pow(G::GENERATOR, r1);
		auto c2 = senc::utils::pow(G::GENERATOR, r2);

		auto z1 = senc::utils::pow(pubKey1, r1);
		auto z2 = senc::utils::pow(pubKey2, r2);

		auto k = this->_kdf(z1, z2);

		auto c3 = this->_symmetricSchema.encrypt(plaintext, k);

		return { c1, c2, c3 };
	}
}
