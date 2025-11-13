/*********************************************************************
 * \file   HybridElGamal2L_impl.hpp
 * \brief  Implementation of `enc::HybridElGamal2L` class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "HybridElGamal2L.hpp"

#include <utility>

namespace senc::utils::enc
{
	template <Group G, Symmetric1L S, ConstCallable<Key<S>, G, G> KDF>
	inline HybridElGamal2L<G, S, KDF>::HybridElGamal2L(S&& symmetricSchema, KDF&& kdf)
		: _symmetricSchema(std::forward<S>(symmetricSchema)), _kdf(std::forward<KDF>(kdf)) { }
}
