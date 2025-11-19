/*********************************************************************
 * \file   shamir.hpp
 * \brief  Header of Shamir secret sharing utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include "Fraction.hpp"
#include "concepts.hpp"
#include "poly.hpp"

namespace senc::utils::shamir
{
	/**
	 * @concept senc::utils::shamir::ShardID
	 * @brief Looks for a typename that can be used as a Shamir shard ID (poly input & output).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ShardID = PolyInput<Self> &&
		(std::integral<Self> || (
			Devisible<Self> && SelfDevisible<Self> && PolyOutput<Self>
		));
	// if ShardID type is integral, we will use `Fraction` as poly output.
	// otherwise, we will use the type itself, assuming it is fully devisible.

	namespace sfinae
	{
		template <ShardID SID>
		struct poly_output { using type = SID; };

		template <std::integral SID>
		struct poly_output<SID> { using type = Fraction<SID>; };
	}

	/**
	 * @typedef senc::utils::shamir::PolyOutput
	 * @brief Type of shamir polynom output for given shard ID type.
	 * @tparam SID Shamir shard ID type.
	 */
	template <ShardID SID>
	using PolyOutput = typename sfinae::poly_output<SID>::type;

	/**
	 * @concept senc::utils::shamir::Secret
	 * @brief Looks for a typename that can be used as a Shamir-shared secret type.
	 * @tparam Self Examined typename.
	 * @tparam SID Shamir shard ID type.
	 */
	template <typename Self, typename SID>
	concept Secret = Addable<Self> && SelfAddable<Self> &&
		PolyCoeff<Self, SID, SID>;
}

#include "shamir_impl.hpp"
