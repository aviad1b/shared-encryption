/*********************************************************************
 * \file   Shamir.hpp
 * \brief  Header of Shamir class (secret sharing utilities).
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <utility>

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
	concept ShamirShardID = PolyInput<Self> && StdHashable<Self> &&
		EqualityComparable<Self> && BoolConvertible<Self>;

	/**
	 * @concept senc::utils::ShamirSecret
	 * @brief Looks for a typename that can be used as a Shamir-shared secret type.
	 * @tparam Self Examined typename.
	 * @tparam SID Shamir shard ID type.
	 */
	template <typename Self, typename SID>
	concept ShamirSecret = (ZeroConstructible<Self> || DefaultConstructibleClass<Self>) && // for utils::sum
		(OneConstructible<Self> || HasIdentity<Self>) && // for utils::product
		std::constructible_from<Self, SID> && // for acurate computations
		Addable<Self> && SelfAddable<Self> &&
		Subtractable<Self> && SelfSubtractable<Self> &&
		Multiplicable<Self> && SelfMultiplicable<Self> &&
		Devisible<Self> && SelfDevisible<Self> &&
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

		ShamirException(const std::string& msg) : Base(msg) {}

		ShamirException(std::string&& msg) : Base(std::move(msg)) {}

		ShamirException(const std::string& msg, const std::string& info) : Base(msg, info) {}

		ShamirException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) {}

		ShamirException(const Self&) = default;

		Self& operator=(const Self&) = default;

		ShamirException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	/**
	 * @class senc::utils::Shamir
	 * @brief Holds static methods for Shamir secret sharing utilities.
	 * @tparam S Shared secret type.
	 * @tparam SID Type used as Shamir shard ID.
	 */
	template <typename S, ShamirShardID SID = std::int32_t>
	requires ShamirSecret<S, SID>
	class Shamir
	{
	public:
		using PackedSecret = ShamirPackedSecret<S>;
		using Threshold = ShamirThreshold;
		using Poly = ShamirPoly<S, SID>;
		using Shard = std::pair<SID, PackedSecret>; // x, poly(x)

		Shamir() = delete;

		/**
		 * @brief Samples a Shamir polynomial for sharing a given secret.
		 * @param secret Secret being shared (to use as first coefficient)..
		 * @param threshold Threshold of units to be able to restore secret (polynom degree).
		 * @param secretSampler A function for sampling more secrets (coefficients).
		 * @return Sampled polynom for Shamir secret sharing.
		 */
		static Poly sample_poly(const S& secret, Threshold threshold, std::function<S()> secretSampler);

		/**
		 * @brief Gets Shamir shard based on sampled polynomial.
		 * @param poly Polynomial sampled for Shamir.
		 * @param shardID Non-zero ID value for Shamir shard.
		 * @return A pair where `first` is polynomial input (shard ID),
		 *		   and `second` is its output for `first`.
		 * @throw ShamirException If `shardID` is zero-equivalent.
		 */
		static Shard make_shard(const Poly& poly, SID shardID);

		/**
		 * @brief Gets Shamir shards based on sampled polynom.
		 * @param poly Polynomial sampled for Shamir.
		 * @param shardsIDs Non-zero unique ID values for Shamir shards.
		 * @return A list of pairs where each `first` is polynomial input (shard ID),
		 *		   and each `second` is its output for its `first`.
		 * @throw ShamirException If `shardsIDs` aren't unique, or if any of them is zero-equivalent.
		 */
		template <std::ranges::input_range R>
		requires std::convertible_to<std::ranges::range_value_t<R>, SID>
		static std::vector<Shard> make_shards(const Poly& poly, R&& shardsIDs);

		/**
		 * @brief Restores a Shamir-shared secret.
		 * @param shards Shamir shards to restore secret from.
		 * @param threshold Original threshold used for sharing.
		 * @return Restores secret.
		 * @throw ShamirException If not enough shards are provided, or shards are invalid, or failed.
		 */
		static S restore_secret(const std::vector<Shard>& shards, Threshold threshold);

	private:
		/**
		 * @brief Gets Lagrange coefficient for a specific shard in a sequence.
		 * @param i Index of shard in sequence.
		 * @param shardsIDs Sequence of shards IDs.
		 * @return Lagrange coefficient of the `i`th shard from `shardsIDs`.
		 * @throw ShamirException If `shardsIDs` are not unique or contain a zero-equivalent.
		 */
		static PackedSecret get_lagrange_coeff(std::size_t i, const std::vector<SID>& shardsIDs);
	};
}

#include "Shamir_impl.hpp"
