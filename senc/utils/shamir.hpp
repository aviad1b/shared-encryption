/*********************************************************************
 * \file   shamir.hpp
 * \brief  Header of Shamir secret sharing utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include "concepts.hpp"
#include "poly.hpp"

namespace senc::utils::shamir
{
	/**
	 * @concept senc::utils::shamir::ShardID
	 * @brief Looks for a typename that can be used as a Shamir shard ID (poly input & output).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept ShardID = PolyInput<Self> && PolyOutput<Self>;
}

#include "shamir_impl.hpp"
