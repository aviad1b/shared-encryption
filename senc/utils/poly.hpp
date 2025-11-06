/*********************************************************************
 * \file   poly.hpp
 * \brief  Contains polynomial-related declarations & concepts.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <functional>
#include <ostream>
#include <vector>
#include "concepts.hpp"
#include "math.hpp"

namespace senc::utils
{
	/**
	 * @typedef senc::utils::PolyDegree
	 * @brief Type used to represent polynomial degree.
	 */
	using PolyDegree = int;

	/**
	 * @concept senc::utils::PolyInput
	 * @brief Looks for a typename that can be used as a polynom input type.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept PolyInput = PowerRaisable<Self>;

	template <typename Self>
	concept AnyPolyCoeff = requires(const Self self)
	{
		{ std::declval<Self>() + std::declval<Self>() } -> std::same_as<Self>;
		{ self += std::declval<Self>() } -> std::same_as<Self&>;
	};

	/**
	 * @concept senc::utils::PolyCoeff
	 * @brief Looks for a typename that can be used as a polynom coefficient.
	 * @tparam Self Examined typename.
	 * @tparam I Polynomial input type.
	 */
	template <typename Self, typename I>
	concept PolyCoeff = AnyPolyCoeff<Self> && requires(const I& x)
	{
		{ std::declval<Self>() * x } -> std::same_as<Self>;
	};

	/**
	 * @class senc::utils::Poly
	 * @brief Represents a polynomial.
	 * @tparam I Polynom input type (must satisfy `senc::utils::PolyInput`).
	 * @tparam C Polynom coefficient type (must satisfy `senc::utils::PolyCoeff`).
	 */
	template <PolyInput I, PolyCoeff<I> C>
	class Poly
	{
	public:
		using Self = Poly<I, C>;

		/**
		 * @brief Constructs a polynomial from a (moved) vector of coefficients.
		 * @param coeffs Polynom coefficients (moved).
		 */
		explicit Poly(std::vector<C>&& coeffs);

		/**
		 * @brief Constructs a polynomial from an initializer list of coefficients.
		 * @param coeffs Polynom coefficients.
		 * @note Requires `C` to be `std::copy_constructible`.
		 */
		explicit Poly(std::initializer_list<C> coeffs)
		requires std::copy_constructible<C>;

		/**
		 * @brief Constructs a polynomial from an input range of coefficients.
		 * @param coeffs Polynom coefficients.
		 * @note Requires `C` to be `std::copy_constructible`.
		 */
		explicit Poly(const InputRange<C> auto& coeffs)
		requires std::copy_constructible<C>;

		/**
		 * @brief Constructs a polynomial from a (moved) input range of coefficients.
		 * @param coeffs Polynom coefficients.
		 * @note Requires `C` to be `std::move_constructible`.
		 */
		explicit Poly(InputRange<C> auto&& coeffs)
		requires std::move_constructible<C>;

		/**
		 * @brief Constructs a polynomial from coefficients.
		 * @param coeffs Polynom coefficients.
		 * @note Requires `C` to be `std::copy_constructible`.
		 */
		template <std::same_as<C>... Cs>
		explicit Poly(const Cs&... coeffs)
		requires std::copy_constructible<C>;

		/**
		 * @brief Constructs a polynomial from moved coefficients.
		 * @param coeffs Polynom coefficients (moved each).
		 * @note Requires `C` to be `std::move_constructible`.
		 */
		template <std::same_as<C>... Cs>
		explicit Poly(Cs&&... coeffs)
		requires std::move_constructible<C>;

		/**
		 * @brief Samples a random polynom using a given coefficient sampler.
		 * @param degree Target polynomial degree.
		 * @param coeffSampler Polynom coefficient sampler function.
		 * @return Sampled polynomial.
		 */
		static Self sample(PolyDegree degree, std::function<C()> coeffSampler);

		/**
		 * @brief Samples a random polynom using a given coefficient sampler.
		 * @param degree Target polynomial degree.
		 * @param coeffSampler Polynom coefficient sampler function.
		 * @param coeffs Existing coefficients (from least significant to most).
		 * @return Sampled polynomial.
		 * @note Requires `C` to be `std::copy_constructible`.
		 */
		template <std::same_as<C>... Cs>
		static Self sample(PolyDegree degree, std::function<C()> coeffSampler, const Cs&... coeffs)
		requires std::copy_constructible<C>;

		/**
		 * @brief Samples a random polynom using a given coefficient sampler.
		 * @param degree Target polynomial degree.
		 * @param coeffSampler Polynom coefficient sampler function.
		 * @param coeffs Existing coefficients (moved each; from least significant to most).
		 * @return Sampled polynomial.
		 * @note Requires `C` to be `std::move_constructible`.
		 */
		template <std::same_as<C>... Cs>
		static Self sample(PolyDegree degree, std::function<C()> coeffSampler, Cs&&... coeffs)
		requires std::move_constructible<C>;

		/**
		 * @brief Gets polynomial degree.
		 * @return Polynomial degree.
		 */
		PolyDegree degree() const;

		/**
		 * @brief Calls polynomial function on a given input.
		 * @param x Input to call polynomial function on.
		 * @return Polynomial result.
		 */
		C operator()(const I& x) const;

	private:
		std::vector<C> _coeffs;

		void sample_missing_coeffs(PolyDegree degree, std::function<C()> coeffSampler);
	};
}

#include "poly_impl.hpp"
