/*********************************************************************
 * \file   poly_impl.hpp
 * \brief  Contains polynomial-related definitions & implementations.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "poly.hpp"
#include <numeric>

namespace senc::utils
{
	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline Poly<I, O, C>::Poly(std::vector<C>&& coeffs) : _coeffs(std::move(coeffs)) { }

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline Poly<I, O, C>::Poly(std::initializer_list<C> coeffs)
	requires std::copy_constructible<C>
		: _coeffs(coeffs) { }

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline Poly<I, O, C>::Poly(const InputRange<C> auto& coeffs)
	requires std::copy_constructible<C>
	{
		auto rng = std::ranges::subrange(
			std::ranges::begin(coeffs),
			std::ranges::end(coeffs)
		);
		this->_coeffs.assign(rng.begin(), rng.end());
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline Poly<I, O, C>::Poly(InputRange<C> auto&& coeffs)
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

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	template <std::same_as<C>... Cs>
	inline Poly<I, O, C>::Poly(const Cs&... coeffs)
	requires std::copy_constructible<C>
		: _coeffs({ coeffs... }) { }

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	template <std::same_as<C>... Cs>
	inline Poly<I, O, C>::Poly(Cs&&... coeffs)
	requires std::move_constructible<C>
	{
		this->_coeffs.reserve(sizeof...(coeffs));
		(this->_coeffs.emplace_back(std::forward<Cs>(coeffs)), ...);
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline Poly<I, O, C>::Self Poly<I, O, C>::sample(PolyDegree degree, std::function<C()> coeffSampler)
	{
		Self res;
		res.sample_missing_coeffs(degree, coeffSampler);
		return res;
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	template <std::same_as<C> ...Cs>
	inline Poly<I, O, C>::Self Poly<I, O, C>::sample(PolyDegree degree, std::function<C()> coeffSampler, const Cs&... coeffs)
	requires std::copy_constructible<C>
	{
		Self res(coeffs...);
		res.sample_missing_coeffs(degree, coeffSampler);
		return res;
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	template <std::same_as<C> ...Cs>
	inline Poly<I, O, C>::Self Poly<I, O, C>::sample(PolyDegree degree, std::function<C()> coeffSampler, Cs && ...coeffs)
	requires std::move_constructible<C>
	{
		Self res(std::move(coeffs)...);
		res.sample_missing_coeffs(degree, coeffSampler);
		return res;
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline PolyDegree Poly<I, O, C>::degree() const
	{
		return this->_coeffs.size() - 1;
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline O Poly<I, O, C>::operator()(const I& x) const
	{
		C res = this->_coeffs[0];
		for (size_t i = 1; i < this->_coeffs.size(); ++i)
			res += this->_coeffs[i] * ::senc::utils::pow(x, static_cast<Exponent>(i));
		return res;
	}

	template <PolyInput I, PolyOutput O, PolyCoeff<I, O> C>
	inline void Poly<I, O, C>::sample_missing_coeffs(PolyDegree degree, std::function<C()> coeffSampler)
	{
		this->_coeffs.reserve(static_cast<std::size_t>(degree) + 1);
		while (this->_coeffs.size() <= degree)
			this->_coeffs.push_back(coeffSampler());
	}
}
