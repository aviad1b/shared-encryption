/*********************************************************************
 * \file   poly_impl.hpp
 * \brief  Contains polynomial-related definitions & implementations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "poly.hpp"

namespace senc::utils
{
	template <typename I, PolyCoeff<I> C>
	inline Poly<I, C>::Poly(std::vector<C>&& coeffs) : _coeffs(std::move(coeffs)) { }

	template <typename I, PolyCoeff<I> C>
	inline Poly<I, C>::Poly(std::initializer_list<C> coeffs)
	requires std::copy_constructible<C>
		: _coeffs(coeffs) { }

	template <typename I, PolyCoeff<I> C>
	inline Poly<I, C>::Poly(const InputRange<C> auto& coeffs)
	requires std::copy_constructible<C>
	{
		auto rng = std::ranges::subrange(
			std::ranges::begin(coeffs),
			std::ranges::end(coeffs)
		);
		this->_coeffs.assign(rng.begin(), rng.end());
	}

	template <typename I, PolyCoeff<I> C>
	inline Poly<I, C>::Poly(InputRange<C> auto&& coeffs)
	requires std::move_constructible<C>
	{
		auto rng = std::ranges::subrange(
			std::make_move_iterator(std::ranges::begin(coeffs)),
			std::make_move_iterator(std::ranges::end(coeffs))
		);
		this->_coeffs.assign(
			std::make_move_iterator(rng.begin()),
			std::make_move_iterator(rng.end())
		);
	}

	template <typename I, PolyCoeff<I> C>
	template <std::same_as<C>... Cs>
	inline Poly<I, C>::Poly(const Cs&... coeffs)
	requires std::copy_constructible<C>
		: _coeffs({ coeffs... }) { }

	template <typename I, PolyCoeff<I> C>
	template <std::same_as<C>... Cs>
	inline Poly<I, C>::Poly(Cs&&... coeffs)
	requires std::move_constructible<C>
	{
		this->_coeffs.reserve(sizeof...(coeffs));
		(this->_coeffs.emplace_back(std::forward<Cs>(args)), ...);
	}
}
