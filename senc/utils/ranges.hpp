/*********************************************************************
 * \file   ranges.hpp
 * \brief  Header of range-related utilities.
 * 
 * \author aviad1b
 * \date   November 2025
 *********************************************************************/

#pragma once

#include <optional>
#include <numeric>
#include <utility>
#include <ranges>

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
	requires Multiplicable<std::ranges::range_value_t<R>>
	std::optional<std::ranges::range_value_t<R>> product(R&& r);

	/**
	 * @brief Computes sum of all element in range.
	 * @param r Range to compute sum of.
	 * @return `std::nullopt` if `r` is empty, elements sum otherwise.
	 */
	template <std::ranges::input_range R>
	requires Addable<std::ranges::range_value_t<R>>
	std::optional<std::ranges::range_value_t<R>> sum(R&& r);

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
}

#include "ranges_impl.hpp"
