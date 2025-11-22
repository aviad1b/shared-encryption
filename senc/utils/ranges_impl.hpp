/*********************************************************************
 * \file   ranges_impl.hpp
 * \brief  Template implementation of range-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ranges.hpp"

namespace senc::utils
{
	template <typename T, std::ranges::input_range R>
	requires std::convertible_to<std::ranges::range_value_t<R>, T>
	inline std::vector<T> to_vector(R&& range)
	{
		std::vector<T> res;
		for (auto it = range.begin(); it != range.end(); ++it)
			res.emplace_back(*it);
		return res;
	}

	template <std::ranges::input_range R>
	requires Multiplicable<std::ranges::range_value_t<R>>
	inline std::optional<std::ranges::range_value_t<R>> product(R&& r)
	{
		auto it = std::ranges::begin(r);
		auto end = std::ranges::end(r);

		if (it == end)
			return std::nullopt;

		auto res = *it;
		++it;

		return std::accumulate(
			it, end, res,
			std::multiplies<std::ranges::range_value_t<R>>{}
		);
	}

	template <std::ranges::input_range R>
	requires Addable<std::ranges::range_value_t<R>> &&
		(ZeroConstructible<std::ranges::range_value_t<R>> || 
			DefaultConstructibleClass<std::ranges::range_value_t<R>>)
	inline std::ranges::range_value_t<R> sum(R&& r)
	{
		auto it = std::ranges::begin(r);
		auto end = std::ranges::end(r);

		if (it == end)
		{
			if constexpr (DefaultConstructibleClass<std::ranges::range_value_t<R>>)
				return std::ranges::range_value_t<R>{};
			else
				return std::ranges::range_value_t<R>(0);
		}

		auto res = *it;
		++it;

		return std::accumulate(
			it, end, res,
			std::plus<std::ranges::range_value_t<R>>{}
		);
	}

	namespace ranges
	{
		template <std::ranges::view V>
		inline EnumerateViewIterator<V>::EnumerateViewIterator(
			std::ranges::iterator_t<V>&& it, std::size_t idx
		) : _it(std::forward<std::ranges::iterator_t<V>>(it)), _idx(idx) { }

		template <std::ranges::view V>
		inline EnumerateViewIterator<V>::value_type EnumerateViewIterator<V>::operator*() const
		{
			return { _idx, *_it };
		}

		template <std::ranges::view V>
		inline EnumerateViewIterator<V>::Self& EnumerateViewIterator<V>::operator++()
		{
			++_it;
			++_idx;
			return *this;
		}

		template <std::ranges::view V>
		inline EnumerateViewIterator<V>::Self EnumerateViewIterator<V>::operator++(int)
		{
			Self res = *this;
			++(*this);
			return res;
		}

		template <std::ranges::view V>
		inline bool EnumerateViewIterator<V>::operator==(const Self& other) const noexcept
		{
			return this->_it == other._it;
		}

		template <std::ranges::view V>
		inline EnumerateView<V>::EnumerateView(V&& wrappedView) : _wrappedView(std::move(wrappedView)) {}

		template <std::ranges::view V>
		inline EnumerateView<V>::iterator EnumerateView<V>::begin()
		{
			return iterator(std::ranges::begin(this->_wrappedView), 0);
		}

		template <std::ranges::view V>
		inline EnumerateView<V>::iterator EnumerateView<V>::end()
		{
			return iterator(std::ranges::end(this->_wrappedView), 0);
			// `0` is a dummy index here, since `operator==` will discard it
		}
	}
}
