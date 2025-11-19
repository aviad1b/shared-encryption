/*********************************************************************
 * \file   Shamir.hpp
 * \brief  Header of Shamir class (secret sharing utilities).
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
	 * @typedef senc::utils::ShamirThreshold
	 * @brief Type used to store threshold of units required for restoring.
	 */
	using ShamirThreshold = PolyDegree;

	/**
	 * @concept senc::utils::ShamirShardID
	 * @brief Looks for a typename that can be used as a Shamir shard ID (poly input & output).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ShamirShardID = PolyInput<Self> && 
		EqualityComparable<Self> && BoolConvertible<Self>;

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
	template <typename S, ShamirShardID SID>
	requires ShamirSecret<S, SID>
	using ShamirPoly = Poly<SID, ShamirPackedSecret<S>, ShamirPackedSecret<S>>;

	/**
	 * @class senc::utils::ShamirException
	 * @brief Type of exceptions thrown on Shamir secret sharing related errors.
	 */
	class ShamirException : public Exception
	{
	public:
		using Self = ShamirException;
		using Base = Exception;

		ShamirException(const std::string& msg) : Base(msg) { }

		ShamirException(std::string&& msg) : Base(std::move(msg)) { }

		ShamirException(const std::string& msg, const std::string& info) : Base(msg, info) { }
		
		ShamirException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) { }

		ShamirException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ShamirException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};
}

#include "shamir_impl.hpp"
