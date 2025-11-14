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
	const Distribution<CryptoPP::Integer> HybridElGamal2L<G, S, KDF>::PRIV_KEY_DIST =
		Random<CryptoPP::Integer>::get_dist_below(G::ORDER);

	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	inline HybridElGamal2L<G, S, KDF>::HybridElGamal2L(S&& symmetricSchema, KDF&& kdf)
		: _symmetricSchema(std::forward<S>(symmetricSchema)), _kdf(std::forward<KDF>(kdf)) { }
	
	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	inline std::pair<HybridElGamal2L<G, S, KDF>::PubKey, HybridElGamal2L<G, S, KDF>::PrivKey>
		HybridElGamal2L<G, S, KDF>::keygen() const
	{
		PrivKey privKey = PRIV_KEY_DIST();
		PubKey pubKey = senc::utils::pow(G::GENERATOR, privKey);
		return { pubKey, privKey };
	}
}
