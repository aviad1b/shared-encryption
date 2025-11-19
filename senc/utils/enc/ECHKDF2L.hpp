/*********************************************************************
 * \file   ECHKDF2L.hpp
 * \brief  Header of ECHKDF2L class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "../ECGroup.hpp"
#include "AES1L.hpp"

namespace senc::utils::enc
{
	/**
	 * @class senc::utils::enc::ECHKDF2L
	 * @brief Default-constructible HKDF functor for Elliptic Curve elements (two-layer).
	 */
	class ECHKDF2L
	{
	public:
		/**
		 * @brief Derives an AES1L key from two ECGroup elements.
		 * @param a First ECGroup element.
		 * @param b Second ECGroup element.
		 * @return Derives AES1L key.
		 */
		AES1L::Key operator()(const ECGroup& a, const ECGroup& b) const;
	};
}
