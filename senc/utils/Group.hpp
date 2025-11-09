/*********************************************************************
 * \file   Group.hpp
 * \brief  Contains definitions related to algebric groups.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <concepts>

namespace senc::utils
{
	/**
	 * @concept senc::utils::Group
	 * @brief Looks for a typename that can be used as an algebric group.
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept Group = std::copyable<Self> && requires(Self self, const Self cself)
	{
		{ std::declval<Self>() * std::declval<Self>() } -> std::same_as<Self>;
		{ self *= std::declval<Self>() } -> std::same_as<Self&>;
		{ std::declval<Self>() / std::declval<Self>() } -> std::same_as<Self>;
		{ self /= std::declval<Self>() } -> std::same_as<Self&>;
		{ cself.inverse() } -> std::same_as<Self>;
		{ Self::GENERATOR } -> std::convertible_to<Self>;
		{ Self::ORDER } -> std::convertible_to<std::size_t>;
	};
}
