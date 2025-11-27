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
	requires Multiplicable<std::ranges::range_value_t<R>> &&
		(OneConstructible<std::ranges::range_value_t<R>> || 
			HasIdentity<std::ranges::range_value_t<R>>)
	inline std::ranges::range_value_t<R> product(R&& r)
	{
		auto it = std::ranges::begin(r);
		auto end = std::ranges::end(r);

		if (it == end)
		{
			if constexpr (HasIdentity<std::ranges::range_value_t<R>>)
				return std::ranges::range_value_t<R>::identity();
			else
				return std::ranges::range_value_t<R>(1);
		}

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

		template <bool isConst, std::ranges::input_range ...Ranges>
		inline ZipViewIterator<isConst, Ranges...>::ZipViewIterator(RangesTuple& ranges, bool isEnd)
			: _ranges(&ranges)
		{
			if (!isEnd)
			{
				// if not end, take begin iterator of all
				// (assuming only directly-retrieved iterators are begin and end, for simplicity)
				this->_its = std::apply(
					[](auto&... its) { return std::tuple{ std::ranges::begin(its)... }; },
					ranges
				);
			}
			else
			{
				// if end, take end iteraor of all
				this->_its = std::apply(
					[](auto&... its) { return std::tuple{ std::ranges::end(its)... }; },
					ranges
				);
			}
		}

		template <bool isConst, std::ranges::input_range ...Ranges>
		inline ZipViewIterator<isConst, Ranges...>::value_type
			ZipViewIterator<isConst, Ranges...>::operator*() const
		{
			return std::apply(
				[](auto&... its) { return value_type(*its...); },
				this->_its
			);
		}

		template <bool isConst, std::ranges::input_range ...Ranges>
		inline ZipViewIterator<isConst, Ranges...>::Self&
			ZipViewIterator<isConst, Ranges...>::operator++()
		{
			// increment all iterators
			std::apply([](auto&... its) { (++its, ...); }, this->_its);

			// if any iterator reached end state, move `this` to end state
			if (any_end(*this->_ranges, this->_its, std::make_index_sequence<sizeof...(Ranges)>{}))
				*this = Self(*this->_ranges, true);
			
			return *this;
		}

		template <std::ranges::input_range... Ranges>
		inline ZipView<Ranges...>::ZipView(Ranges&&... ranges)
			: _ranges(std::forward<Ranges>(ranges)...) { }

		template <std::ranges::input_range... Ranges>
		inline ZipView<Ranges...>::iterator ZipView<Ranges...>::begin()
		{
			return iterator(this->_ranges);
		}

		template <std::ranges::input_range... Ranges>
		inline ZipView<Ranges...>::iterator ZipView<Ranges...>::end()
		{
			return iterator(this->_ranges, true); // isEnd = true
		}

		template <std::ranges::input_range... Ranges>
		inline ZipView<Ranges...>::const_iterator ZipView<Ranges...>::begin() const
		{
			return const_iterator(this->_ranges);
		}

		template <std::ranges::input_range... Ranges>
		inline ZipView<Ranges...>::const_iterator ZipView<Ranges...>::end() const
		{
			return const_iterator(this->_ranges, true); // isEnd = true
		}
	}
}
