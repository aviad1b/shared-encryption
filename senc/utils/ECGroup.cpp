/*********************************************************************
 * \file   ECGroup.cpp
 * \brief  Implementation of ECGroup class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ECGroup.hpp"

#include <sstream>
#include "StrParseException.hpp"

namespace senc::utils
{
	GroupOrder ECGroup::order()
	{
		return ec_params().GetSubgroupOrder();
	}

	ECGroup::Self ECGroup::generator()
	{
		static const auto GENERATOR = Self(ec_base_point());
		return GENERATOR;
	}

	ECGroup::Self ECGroup::identity()
	{
		static const auto IDENTITY = Self(true); // isIdentity=true
		return IDENTITY;
	}

	ECGroup::ECGroup() : Self(true) { } // isIdentity = true

	ECGroup::ECGroup(const BigInt& x, const BigInt& y)
		: _point(x, y), _isIdentity(false) { }

	ECGroup::ECGroup(BigInt&& x, BigInt&& y)
		: _point(std::move(x), std::move(y)), _isIdentity(false) { }

	ECGroup::Self ECGroup::from_scalar(const BigInt& scalar)
	{
		if (scalar.IsZero())
			return identity();
		return Self(ec_curve().Multiply(scalar, ec_base_point()));
	}

	ECGroup::Self ECGroup::sample()
	{
		return from_scalar(dist()());
	}

	ECGroup::Self ECGroup::from_bytes(const Buffer& bytes)
	{
		const bigint_size_t xSize = *reinterpret_cast<const bigint_size_t*>(
			bytes.data()
		);
		if (!xSize) // if no x size, means no contents, means identity
			return identity();

		const bigint_size_t ySize = *reinterpret_cast<const bigint_size_t*>(
			bytes.data() + sizeof(bigint_size_t)
		);

		const byte* data = bytes.data() + (2 * sizeof(bigint_size_t)); // after sizes
		BigInt x, y;
		x.Decode(data, xSize);
		y.Decode(data + xSize, ySize);

		return Self(std::move(x), std::move(y));
	}

	Buffer ECGroup::to_bytes() const
	{
		if (is_identity())
			return Buffer(1, 0); // size 1, value 0

		// buffer will contain x size then y size then x then y
		Buffer res((2 * sizeof(bigint_size_t)) + x().MinEncodedSize() + y().MinEncodedSize());
		bigint_size_t* pXSize = reinterpret_cast<bigint_size_t*>(res.data());
		bigint_size_t* pYSize = pXSize + 1;
		byte* pX = reinterpret_cast<byte*>(pYSize + 1);
		byte* pY = pX + x().MinEncodedSize();

		*pXSize = x().MinEncodedSize();
		*pYSize = y().MinEncodedSize();
		x().Encode(pX, x().MinEncodedSize());
		y().Encode(pY, y().MinEncodedSize());

		return res;
	}

	ECGroup::Self ECGroup::decode(const Buffer& bytes)
	{
		if (bytes.size() != ENCODED_SIZE)
			throw std::invalid_argument("Failed to decode group element: Invalid encoded point size");

		if (0x00 == bytes[0])
			return identity();

		if (0x02 != bytes[0] && 0x03 != bytes[0])
			throw std::invalid_argument("Failed to decode group element: Invalid point prefix byte");

		BigInt x{};
		x.Decode(bytes.data() + 1, ENCODED_FIELD_SIZE);

		// recover Y from X using the curve equation using CryptoPP's ECP DecodePoint
		Buffer sec1(ENCODED_SIZE);
		std::memcpy(sec1.data(), bytes.data(), ENCODED_SIZE);

		ECP::Point point{};
		if (!ec_curve().DecodePoint(point, sec1.data(), ENCODED_SIZE))
			throw std::invalid_argument("Failed to decode group element: Point is not on the curve");

		return Self(point);
	}

	Buffer ECGroup::encode() const
	{
		Buffer res(ENCODED_SIZE, 0);

		if (is_identity())
		{
			res[0] = 0x00;
			return res;
		}

		// prefix: 0x02 if Y even, 0x03 if Y odd
		res[0] = y().IsOdd() ? 0x03 : 0x02;
		x().Encode(res.data() + 1, ENCODED_FIELD_SIZE); // zero-pads on the left automatically
		return res;
	}

	ECGroup::Self ECGroup::from_string(std::string str)
	{
		static const std::string PREFIX = "ECGroup(";
		static const std::string SUFFIX = ")";

		// check for prefix and suffix, then trim them off
		if (!str.starts_with(PREFIX) || !str.ends_with(SUFFIX))
			throw StrParseException("Invalid input", str);
		str = str.substr(PREFIX.length());
		str = str.substr(0, str.length() - SUFFIX.length());

		// check for identity representative
		if ("IDENTITY" == str)
			return identity();

		auto commaIdx = str.find(',');
		if (commaIdx == std::string::npos)
			throw StrParseException("Invalid input", str);

		auto xStr = str.substr(0, commaIdx);
		auto yStr = str.substr(commaIdx + 1);
		return Self(BigInt(xStr.c_str()), BigInt(yStr.c_str()));
	}

	std::string ECGroup::to_string() const
	{
		std::stringstream s;
		s << *this;
		return s.str();
	}

	std::ostream& operator<<(std::ostream& os, const ECGroup& elem)
	{
		if (elem.is_identity())
			return os << "ECGroup(IDENTITY)";
		return os << "ECGroup(" << elem._point.x << "," << elem._point.y << ")";
	}

	bool ECGroup::is_identity() const
	{
		return this->_isIdentity;
	}

	bool ECGroup::operator==(const Self& other) const
	{
		if (this->is_identity() && other.is_identity())
			return true;
		if (this->is_identity() || other.is_identity())
			return false;
		return (this->_point.x == other._point.x && this->_point.y == other._point.y);
	}

	const BigInt& ECGroup::x() const
	{
		return this->_point.x;
	}

	const BigInt& ECGroup::y() const
	{
		return this->_point.y;
	}

	ECGroup::Self ECGroup::inverse() const
	{
		if (this->is_identity())
			return identity();
		BigInt p = ec_curve().GetField().GetModulus();
		BigInt negY = (p - this->_point.y) % p;
		return Self(this->_point.x, negY);
	}

	ECGroup::Self ECGroup::operator*(const Self& other) const
	{
		if (this->is_identity())
			return other;
		if (other.is_identity())
			return *this;
		return Self(ec_curve().Add(this->_point, other._point));
	}

	ECGroup::Self& ECGroup::operator*=(const Self& other)
	{
		if (other.is_identity())
			return *this;
		if (this->is_identity())
			return *this = other;
		this->_point = ec_curve().Add(this->_point, other._point);
		return *this;
	}

	ECGroup::Self ECGroup::operator/(const Self& other) const
	{
		return *this * other.inverse();
	}

	ECGroup::Self& ECGroup::operator/=(const Self& other)
	{
		return *this *= other.inverse();
	}

	ECGroup::Self ECGroup::pow(const BigInt& exp) const
	{
		if (this->is_identity() || exp.IsZero())
			return identity();

		if (exp.IsNegative())
			return this->inverse().pow(-exp);

		return Self(ec_curve().Multiply(exp, this->_point));
	}

	Distribution<BigInt>& ECGroup::dist()
	{
		static auto DIST = Random<BigInt>::get_dist_below(order());
		return DIST;
	}

	const CryptoPP::DL_GroupParameters_EC<ECGroup::ECP>& ECGroup::ec_params()
	{
		static const auto EC_PARAMS = CryptoPP::DL_GroupParameters_EC<ECP>(CryptoPP::ASN1::secp256r1());
		return EC_PARAMS;
	}

	const ECGroup::ECP& ECGroup::ec_curve()
	{
		static const ECP EC_CURVE = ec_params().GetCurve();
		return EC_CURVE;
	}

	const ECGroup::Point& ECGroup::ec_base_point()
	{
		static const Point EC_BASE_POINT = ec_params().GetSubgroupGenerator();
		return EC_BASE_POINT;
	}

	ECGroup::ECGroup(bool isIdentity) : _isIdentity(isIdentity) { }

	ECGroup::ECGroup(const Point& point) : _point(point), _isIdentity(false) { }
}
