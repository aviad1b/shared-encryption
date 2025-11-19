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
		using Self = ECHKDF2L;

		/**
		 * @brief Constructs an ECHKDF2L instance with default parameters.
		 */
		ECHKDF2L();

		/**
		 * @brief Constructs an ECHKDF2L instance with given allocated size for each group element in IKM.
		 * @param ikmEachSize Size to use in input keying material for each group element.
		 */
		ECHKDF2L(std::size_t ikmEachSize);

		/**
		 * @brief Constructs an ECHKDF2L wiht given salt value.
		 * @param salt Salt value.
		 */
		ECHKDF2L(const Buffer& salt);

		/**
		 * @brief Constructs an ECHKDF2L instance with given parameter values.
		 * @param ikmEachSize Size to use in input keying material for each group element.
		 * @param salt Salt value.
		 */
		ECHKDF2L(std::size_t ikmEachSize, const Buffer& salt);

		/**
		 * @brief Derives an AES1L key from two ECGroup elements.
		 * @param a First ECGroup element.
		 * @param b Second ECGroup element.
		 * @return Derives AES1L key.
		 */
		AES1L::Key operator()(const ECGroup& a, const ECGroup& b) const;

	private:
		std::size_t _ikmEachSize;
		Buffer _salt;

		static constexpr std::size_t DEFAULT_IKM_EACH_SIZE = 32; // default ikmEachSize is 32 (resulting 64-byte IKM)
		static constexpr std::initializer_list<byte> DEFAULT_SALT = { 4, 3, 5 }; // default salt value
	};
}
