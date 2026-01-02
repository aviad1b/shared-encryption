/*********************************************************************
 * \file   ECGroup.hpp
 * \brief  Header of ECGroup class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#pragma once

#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>
#include <cryptopp/ecp.h>
#include <ostream>

#include "Random.hpp"
#include "Group.hpp"
#include "bytes.hpp"
#include "math.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::ECGroup
	 * @brief Elliptic Curve based algebric group.
	 * @note Satisfies `senc::utils::Group`.
	 */
	class ECGroup
	{
	public:
		using Self = ECGroup;

		static constexpr bool is_prime_ordered() noexcept { return true; } // EC is always prime ordered

		/**
		 * @brief Gets group order.
		 * @return Group order.
		 */
		static GroupOrder order();

		/**
		 * @brief Gets group generator.
		 * @return Group generator.
		 */
		static Self generator();

		/**
		 * @brief Gets group identity element.
		 * @return Group identity element.
		 */
		static Self identity();

		/**
		 * @brief Constructs group identity element.
		 */
		ECGroup();

		/**
		 * @brief Constructs a non-identity group element from given x and y values on the curve.
		 * @param x
		 * @param y
		 */
		ECGroup(const BigInt& x, const BigInt& y);

		/**
		 * @brief Constructs a non-identity group element from given moved x and y values.
		 * @param x
		 * @param y
		 */
		ECGroup(BigInt&& x, BigInt&& y);

		/**
		 * @brief Copy constructor of elliptic curve group element.
		 */
		ECGroup(const Self&) = default;

		/**
		 * @brief Copy assignment operator of elliptic curve group element.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of elliptic curve group element.
		 */
		ECGroup(Self&&) = default;

		/**
		 * @brief Move assignment operator of elliptic curve group element.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Maps given scalar to an elliptic curve group element.
		 * @param scalar Scalar to convert to ECGroup elem (in range [`0`, `ORDER`]).
		 * @return Elliptic curve group mapped from `scalar`.
		 */
		static Self from_scalar(const BigInt& scalar);

		/**
		 * @brief Samples a random group element.
		 * @return Sampled element.
		 */
		static Self sample();

		/**
		 * @brief Parses group element from bytes.
		 * @param bytes Buffer of bytes.
		 * @return Parsed group element.
		 */
		static Self from_bytes(const Buffer& bytes);

		/**
		 * @brief Serializes group element to bytes.
		 * @return Serialized group element.
		 */
		Buffer to_bytes() const;

		/**
		 * @brief Parses group element from string.
		 * @param str String to parse.
		 * @return Parsed group element.
		 */
		static Self from_string(std::string str);

		/**
		 * @brief Converts group element to string.
		 * @return String representation of group element.
		 */
		std::string to_string() const;

		/**
		 * @brief Output operator for `ECGroup`.
		 */
		friend std::ostream& operator<<(std::ostream& os, const ECGroup& elem);

		/**
		 * @brief Determines whether or not this is the identity element.
		 * @return `true` if `*this` is the identity element, otherwise `false`.
		 */
		bool is_identity() const;

		/**
		 * @brief Compares two elliptic curve group elements.
		 * @param other Other element to compare with.
		 * @return `true` if `*this` is the same element as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const;

		/**
		 * @brief Gets point's X value.
		 * @return Point's X value.
		 */
		const BigInt& x() const;

		/**
		 * @brief Gets point's Y value.
		 * @return Point's Y value.
		 */
		const BigInt& y() const;

		/**
		 * @brief Gets inverse of group element (by group operation).
		 * @return Inverse element of `*this`.
		 */
		Self inverse() const;

		/**
		 * @brief Applies group operation (point addition).
		 * @param other Other group element to apply operation with.
		 * @return Result of group operation on `*this` and `other`.
		 */
		Self operator*(const Self& other) const;

		/**
		 * @brief Applies group operation (point addition) into this group element.
		 * @param other Other group element to apply operation with.
		 * @return `*this`, after operator applying.
		 */
		Self& operator*=(const Self& other);

		/**
		 * @brief Applies inverse group operation.
		 * @param other Other group element to apply operation with.
		 * @return Result of inverse group operation on `*this` and `other`.
		 */
		Self operator/(const Self& other) const;

		/**
		 * @brief Applies inverse group operation into this group element.
		 * @param other Other group element to apply inverse operation with.
		 * @return `*this`, after operation applying.
		 */
		Self& operator/=(const Self& other);

		/**
		 * @brief Applies repeated group operation.
		 * @param exp Amount of times to apply group operation (plus one).
		 * @return Result of repeated group operation.
		 */
		Self pow(const BigInt& exp) const;

	private:
		using ECP = CryptoPP::ECP;
		using Point = ECP::Point;

		// static constants
		static Distribution<BigInt>& dist();                            // distribution for sampling
		static const CryptoPP::DL_GroupParameters_EC<ECP>& ec_params(); // eliptic curve parameters
		static const ECP& ec_curve();                                   // elliptic curve itself
		static const Point& ec_base_point();                            // base point of curve

		// instance fields
		Point _point;
		bool _isIdentity;

		// private methods

		/**
		 * @brief Constructs instance with given value for `isIdentity`.
		 * @param isIdentity
		 */
		explicit ECGroup(bool isIdentity);

		/**
		 * @brief Constructs a non-identity element form a given point.
		 * @param point
		 */
		explicit ECGroup(const Point& point);
	};

	static_assert(Group<ECGroup>, "senc::utils::ECGroup should satisfy senc::utils::Group");
	static_assert(PrimeOrderedGroup<ECGroup>, "senc::utils::ECGroup should satisfy senc::utils::PrimeOrderedGroup");
}
