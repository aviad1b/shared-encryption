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
	template <typename T, typename R>
	inline std::vector<T> to_vector(R&& range)
	{
		std::vector<T> res;
		for (auto it = range.begin(); it != range.end(); ++it)
			res.emplace_back(*it);
		return res;
	}

	template <typename T, typename R>
	std::set<T> to_ordered_set(R&& range)
	{
		std::set<T> res;
		for (auto it = range.begin(); it != range.end(); ++it)
			res.emplace(*it);
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
		inline StringViewRangeIterator::StringViewRangeIterator()
			: _r(nullptr) { }

		inline StringViewRangeIterator::StringViewRangeIterator(StringViewRange& r)
			: _r(&r)
		{
			++*this;
		}

		inline StringViewRangeIterator::Self& StringViewRangeIterator::operator++()
		{
			if (!_r)
				return *this; // already at end

			if (_r->next(_strv)) // increment and check if reached end
				_r = nullptr;

			return *this;
		}

		inline std::string_view StringViewRangeIterator::operator*() const
		{
			return _strv;
		}

		inline bool StringViewRangeIterator::operator==(std::default_sentinel_t) const
		{
			return !_r; // reached end iff _r is nullptr
		}

		inline StringViewRange::iterator StringViewRange::begin()
		{
			return iterator(*this);
		}

		inline StringViewRange::sentinel StringViewRange::end()
		{
			return std::default_sentinel;
		}

		template <std::ranges::input_range R>
		requires std::convertible_to<std::ranges::range_reference_t<R>, std::string_view>
		inline StringRangeAdapter<R>::StringRangeAdapter(R& r)
			: _it(std::ranges::begin(r)), _end(std::ranges::end(r)) { }

		template <std::ranges::input_range R>
		requires std::convertible_to<std::ranges::range_reference_t<R>, std::string_view>
		inline bool StringRangeAdapter<R>::next(std::string_view& out)
		{
			if (_it == _end)
				return true;
			out = *_it++;
			return false;
		}

		template <std::ranges::view V>
		inline EnumerateViewIterator<V>::EnumerateViewIterator(
			std::ranges::iterator_t<V>&& it, std::size_t idx
		) : _it(std::forward<std::ranges::iterator_t<V>>(it)), _idx(idx) { }

		template <std::ranges::view V>
		inline auto EnumerateViewIterator<V>::operator*() const -> value_type
		{
			return value_type{ _idx, *_it };
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
		template <typename V_>
		inline EnumerateView<V>::EnumerateView(V_&& wrappedView)
			: _wrappedView(std::forward<V_>(wrappedView)) {}

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
		inline auto ZipViewIterator<isConst, Ranges...>::operator*() const -> reference
		{
			return std::apply(
				[](auto&... its) -> reference { return reference(*its...); },
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

		template <bool isConst, std::ranges::input_range ...Ranges>
		inline ZipViewIterator<isConst, Ranges...>::Self
			ZipViewIterator<isConst, Ranges...>::operator++(int)
		{
			Self res = *this;
			++(*this);
			return res;
		}

		template <bool isConst, std::ranges::input_range ...Ranges>
		inline bool ZipViewIterator<isConst, Ranges...>::operator==(const Self& other) const
		{
			return this->_its == other._its;
		}

		template <std::ranges::input_range... Ranges>
		template <typename... Ranges_>
		inline ZipView<Ranges...>::ZipView(Ranges_&&... ranges)
			: _ranges(std::forward<Ranges_>(ranges)...) { }

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

		template <bool isConst, std::ranges::range R1, std::ranges::range R2>
		requires std::common_reference_with<
			std::ranges::range_reference_t<R1>,
			std::ranges::range_reference_t<R2>
		>
		inline ConcatViewIterator<isConst, R1, R2>::ConcatViewIterator(
			bool inFirst, It1 it1, It1 end1, It2 it2)
			: _inFirst(inFirst), _it1(it1), _end1(end1), _it2(it2) { }

		template <bool isConst, std::ranges::range R1, std::ranges::range R2>
		requires std::common_reference_with<
			std::ranges::range_reference_t<R1>,
			std::ranges::range_reference_t<R2>
		>
		inline typename ConcatViewIterator<isConst, R1, R2>::reference
			ConcatViewIterator<isConst, R1, R2>::operator*() const
		{
			return this->_inFirst ? *this->_it1 : *this->_it2;
		}

		template <bool isConst, std::ranges::range R1, std::ranges::range R2>
		requires std::common_reference_with<
			std::ranges::range_reference_t<R1>,
			std::ranges::range_reference_t<R2>
		>
		inline typename ConcatViewIterator<isConst, R1, R2>::Self&
			ConcatViewIterator<isConst, R1, R2>::operator++()
		{
			if (this->_inFirst)
				this->_inFirst = ++this->_it1 != this->_end1;
			else
				++this->_it2;
			return *this;
		}

		template <bool isConst, std::ranges::range R1, std::ranges::range R2>
		requires std::common_reference_with<
			std::ranges::range_reference_t<R1>,
			std::ranges::range_reference_t<R2>
		>
		inline typename ConcatViewIterator<isConst, R1, R2>::Self
			ConcatViewIterator<isConst, R1, R2>::operator++(int)
		{
			Self res = *this;
			++(*this);
			return res;
		}

		template <bool isConst, std::ranges::range R1, std::ranges::range R2>
		requires std::common_reference_with<
			std::ranges::range_reference_t<R1>,
			std::ranges::range_reference_t<R2>
		>
		template <bool otherIsConst>
		inline bool ConcatViewIterator<isConst, R1, R2>::operator==(const ConcatViewIterator<otherIsConst, R1, R2>& other) const
		{
			if (this->_inFirst != other._inFirst)
				return false;
			return this->_inFirst ? this->_it1 == other._it1
				: this->_it2 == other._it2;
		}

		template <std::ranges::range R1, std::ranges::range... Rs>
		template <typename R1_, typename... Rs_>
		requires (sizeof...(Rs_) > 0)
		inline JoinView<R1, Rs...>::JoinView(R1_&& r1, Rs_&&... rs)
			: Base(std::forward<R1_>(r1), JoinView<Rs...>(std::forward<Rs_>(rs)...)) { }

		template <std::ranges::range R1, std::ranges::range R2>
		template <typename R1_, typename R2_>
		inline JoinView<R1, R2>::JoinView(R1_&& r1, R2_&& r2)
			: _r1(std::forward<R1_>(r1)), _r2(std::forward<R2_>(r2)) { }

		template <std::ranges::range R1, std::ranges::range R2>
		inline typename JoinView<R1, R2>::iterator JoinView<R1, R2>::begin()
		{
			const auto firstBegin = std::ranges::begin(this->_r1);
			const auto firstEnd = std::ranges::end(this->_r1);
			const auto secondBegin = std::ranges::begin(this->_r2);
			const bool inFirst = (firstBegin != firstEnd); // begin is in first iff first isn't empty
			return iterator(
				inFirst,
				firstBegin,
				firstEnd,
				secondBegin
			);
		}

		template <std::ranges::range R1, std::ranges::range R2>
		inline typename JoinView<R1, R2>::iterator JoinView<R1, R2>::end()
		{
			return iterator(
				false, //inFirst=false
				std::ranges::end(this->_r1),
				std::ranges::end(this->_r1),
				std::ranges::end(this->_r2)
			);
		}

		template <std::ranges::range R1, std::ranges::range R2>
		inline typename JoinView<R1, R2>::const_iterator JoinView<R1, R2>::begin() const
		{
			const auto firstBegin = std::ranges::begin(this->_r1);
			const auto firstEnd = std::ranges::end(this->_r1);
			const auto secondBegin = std::ranges::begin(this->_r2);
			const bool inFirst = (firstBegin != firstEnd); // begin is in first iff first isn't empty
			return const_iterator(
				inFirst,
				firstBegin,
				firstEnd,
				secondBegin
			);
		}

		template <std::ranges::range R1, std::ranges::range R2>
		inline typename JoinView<R1, R2>::const_iterator JoinView<R1, R2>::end() const
		{
			return const_iterator(
				false, //inFirst=false
				std::ranges::end(this->_r1),
				std::ranges::end(this->_r1),
				std::ranges::end(this->_r2)
			);
		}
	}
}
