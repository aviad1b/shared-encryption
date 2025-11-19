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
	concept ShardID = PolyInput<Self>;

	/**
	 * @concept senc::utils::shamir::Secret
	 * @brief Looks for a typename that can be used as a Shamir-shared secret type.
	 * @tparam Self Examined typename.
	 * @tparam SID Shamir shard ID type.
	 */
	template <typename Self, typename SID>
	concept Secret = Addable<Self> && SelfAddable<Self> &&
		PolyOutput<Self> && PolyCoeff<Self, SID, Self>;
	// in out shamir polynom, secret is coefficients and output; shard ID is input

	namespace sfinae
	{
		template <typename S>
		struct packed_secret { using type = S; };

		template <std::integral S>
		struct packed_secret<S> { using type = Fraction<S>; };
	}

	/**
	 * @typedef senc::utils::shamir::PackedSecret
	 * @brief Secret packed by Shamir utilities for sharing.
	 * @tparam S Original secret shared type. 
	 */
	template <typename S>
	using PackedSecret = typename sfinae::packed_secret<S>::type;

	/**
	 * @typedef senc::utils::shamir::Poly
	 * @brief Polynomial type used for Shamir secret sharing.
	 * @tparam S Type of shared secret.
	 * @tparam SID Type used as Shamir shard ID.
	 */
	template <typename S, ShardID SID = std::int32_t>
	requires Secret<S, SID>
	using Poly = senc::utils::Poly<SID, PackedSecret<S>, PackedSecret<S>>;
}

#include "shamir_impl.hpp"
