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

namespace senc::utils
{
	/**
	 * @concept senc::utils::ShamirShardID
	 * @brief Looks for a typename that can be used as a Shamir shard ID (poly input & output).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ShamirShardID = PolyInput<Self>;

	/**
	 * @concept senc::utils::ShamirSecret
	 * @brief Looks for a typename that can be used as a Shamir-shared secret type.
	 * @tparam Self Examined typename.
	 * @tparam SID Shamir shard ID type.
	 */
	template <typename Self, typename SID>
	concept ShamirSecret = Addable<Self> && SelfAddable<Self> &&
		PolyOutput<Self> && PolyCoeff<Self, SID, Self>;
	// in out shamir polynom, secret is coefficients and output; shard ID is input

	namespace sfinae
	{
		template <typename S>
		struct shamir_packed_secret { using type = S; };

		template <std::integral S>
		struct shamir_packed_secret<S> { using type = Fraction<S>; };
	}

	/**
	 * @typedef senc::utils::ShamirPackedSecret
	 * @brief Secret packed by Shamir utilities for sharing.
	 * @tparam S Original secret shared type. 
	 */
	template <typename S>
	using ShamirPackedSecret = typename sfinae::shamir_packed_secret<S>::type;

	/**
	 * @typedef senc::utils::ShamirPoly
	 * @brief Polynomial type used for Shamir secret sharing.
	 * @tparam S Type of shared secret.
	 * @tparam SID Type used as Shamir shard ID.
	 */
	template <typename S, ShamirShardID SID = std::int32_t>
	requires ShamirSecret<S, SID>
	using ShamirPoly = senc::utils::Poly<SID, ShamirPackedSecret<S>, ShamirPackedSecret<S>>;
}

#include "shamir_impl.hpp"
