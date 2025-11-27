/*********************************************************************
 * \file   ranges.hpp
 * \brief  Header of range-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025
 *********************************************************************/

#pragma once

#include <numeric>
#include <utility>
#include <ranges>
#include <vector>

#include "concepts.hpp"

namespace senc::utils
{
	/**
	 * @brief Converts range into vector.
	 * @tparam T Element type.
	 * @param range Range to convert into vector.
	 * @return Vector version of `range`.
	 */
	template <typename T, std::ranges::input_range R>
	requires std::convertible_to<std::ranges::range_value_t<R>, T>
	std::vector<T> to_vector(R&& range);

	/**
	 * @brief Computes product of all elements in range.
	 * @param r Range to compute product of.
	 * @return `std::nullopt` if `r` is empty, elements product otherwise.
	 */
	template <std::ranges::input_range R>
	requires Multiplicable<std::ranges::range_value_t<R>> &&
		(OneConstructible<std::ranges::range_value_t<R>> || 
			HasIdentity<std::ranges::range_value_t<R>>)
	std::ranges::range_value_t<R> product(R&& r);

	/**
	 * @brief Computes sum of all element in range.
	 * @param r Range to compute sum of.
	 * @return Default-constructed or zero-constructed object if `r` is empty,
	 *         elements sum otherwise.
	 */
	template <std::ranges::input_range R>
	requires Addable<std::ranges::range_value_t<R>> &&
		(ZeroConstructible<std::ranges::range_value_t<R>> || 
			DefaultConstructibleClass<std::ranges::range_value_t<R>>)
	std::ranges::range_value_t<R> sum(R&& r);

	namespace ranges
	{
		/**
		 * @class senc::utils::ranges::EnumerateViewIterator
		 * @brief Iterator type of `senc::utils::ranges::EnumerateView`.
		 * @tparam V Wrapped view type.
		 */
		template <std::ranges::view V>
		class EnumerateViewIterator
		{
		public:
			using Self = EnumerateViewIterator;

			using value_type = std::pair<std::size_t, std::ranges::range_reference_t<V>>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::input_iterator_tag;
			using iterator_concept = std::input_iterator_tag;

			EnumerateViewIterator() = default;

			EnumerateViewIterator(const Self&) = default;

			Self& operator=(const Self&) = default;

			EnumerateViewIterator(Self&&) = default;

			Self& operator=(Self&&) = default;

			explicit EnumerateViewIterator(std::ranges::iterator_t<V>&& it, std::size_t idx);

			value_type operator*() const;

			Self& operator++();

			Self operator++(int);

			bool operator==(const Self& other) const noexcept;

		private:
			std::ranges::iterator_t<V> _it;
			std::size_t _idx;
		};

		/**
		 * @class senc::utils::ranges::EnumerateView
		 * @brief View range allowing indexed iteration.
		 * @tparam V Wrapped view type.
		 */
		template <std::ranges::view V>
		class EnumerateView : public std::ranges::view_interface<EnumerateView<V>>
		{
		public:
			using Self = EnumerateView<V>;

			using iterator = EnumerateViewIterator<V>;

			EnumerateView() = default;

			EnumerateView(const Self&) = default;

			Self& operator=(const Self&) = default;

			EnumerateView(Self&&) = default;

			Self& operator=(Self&&) = default;

			explicit EnumerateView(V&& wrappedView);

			iterator begin();

			iterator end();

		private:
			V _wrappedView;
		};

		// if given range, construct with an all view
		template <std::ranges::range R>
		EnumerateView(R&&) -> EnumerateView<std::views::all_t<R>>;

		/**
		 * @class senc::utils::ranges::EnumerateFn
		 * @brief Closure type for `senc::utils::ranges::EnumerateView`.
		 */
		class EnumerateFn
		{
		public:
			using Self = EnumerateFn;

			EnumerateFn() noexcept = default;

			template <std::ranges::range R>
			constexpr auto operator()(R&& range) const
			{
				return EnumerateView(std::forward<R>(range));
			}
		};

		/**
		 * @brief Allows simple pipe syntax for range and `EnumerateView`.
		 */
		template <std::ranges::range R>
		inline auto operator|(R&& range, EnumerateFn fn)
		{
			return fn(std::forward<R>(range));
		}
	}

	namespace views
	{
		/**
		 * @brief Enumerate view instance.
		 */
		inline constexpr ranges::EnumerateFn enumerate;
	}

	namespace ranges
	{
		/**
		 * @class senc::utils::ranges::ZipViewIterator
		 * @brief Iterator type of `senc::utils::ranges::ZipView`.
		 * @tparam isConst Whether iterator is over constants or not.
		 * @tparam Ranges Wrapped ranges types.
		 */
		template <bool isConst, std::ranges::input_range... Ranges>
		class ZipViewIterator
		{
		public:
			using Self = ZipViewIterator<isConst, Ranges...>;
			using RangesTuple = std::tuple<Ranges...>;
			using ItsTuple = std::tuple<std::ranges::iterator_t<std::conditional_t<isConst, const Ranges, Ranges>>...>;

			using value_type = std::tuple<std::ranges::range_reference_t<Ranges>...>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::input_iterator_tag;
			using iterator_concept = std::input_iterator_tag;

			ZipViewIterator() = default;

			ZipViewIterator(const Self&) = default;

			Self& operator=(const Self&) = default;

			ZipViewIterator(Self&&) = default;

			Self& operator=(Self&&) = default;

			explicit ZipViewIterator(RangesTuple& ranges, bool isEnd = false);

			value_type operator*() const;

			Self& operator++();

			Self operator++(int);

			bool operator==(const Self& other) const noexcept;

		private:
			RangesTuple* _ranges;
			ItsTuple _its;

			template <std::size_t... I>
			static bool any_end(const RangesTuple& rngTpl, const ItsTuple& iterTpl, std::index_sequence<I...>)
			{
				return (... || (std::get<I>(iterTpl) == std::ranges::end(std::get<I>(rngTpl))));
			}
		};

		/**
		 * @class senc::utils::ranges::ZipView
		 * @brief View range allowing simultanious iteration.
		 * @tparam Ranges Wrapped ranges types.
		 */
		template <std::ranges::input_range... Ranges>
		class ZipView : public std::ranges::view_interface<ZipView<Ranges...>>
		{
		public:
			using Self = ZipView<Ranges...>;
			using RangesTuple = std::tuple<Ranges...>;

			using iterator = ZipViewIterator<false, Ranges...>;
			using const_iterator = ZipViewIterator<true, Ranges...>;

			ZipView() = default;

			ZipView(const Self&) = default;

			Self& operator=(const Self&) = default;

			ZipView(Self&&) = default;

			Self& operator=(Self&&) = default;

			explicit ZipView(Ranges&&... ranges);

			iterator begin();

			iterator end();

			const_iterator begin() const;

			const_iterator end() const;

		private:
			RangesTuple _ranges;
		};

		// if given ranges, construct with an all view
		template <std::ranges::range... Ranges>
		ZipView(Ranges&&...) -> ZipView<std::views::all_t<Ranges>...>;

		/**
		 * @class senc::utils::ranges::ZipFn
		 * @brief Closure type for `senc::utils::ranges::ZipView`.
		 */
		class ZipFn
		{
		public:
			using Self = ZipFn;

			ZipFn() noexcept = default;

			template <std::ranges::range... Ranges>
			constexpr auto operator()(Ranges&&... ranges) const
			{
				return ZipView(std::forward<Ranges>(ranges)...);
			}
		};
	}

	namespace views
	{
		/**
		 * @brief Zip view instance.
		 */
		inline constexpr ranges::ZipFn zip;
	}
}

#include "ranges_impl.hpp"
