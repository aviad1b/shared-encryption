/*********************************************************************
 * \file   ECGroup.cpp
 * \brief  Implementation of ECGroup class.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include "ECGroup.hpp"

namespace senc::utils
{
	const CryptoPP::DL_GroupParameters_EC<ECGroup::ECP> ECGroup::EC_PARAMS = 
		CryptoPP::DL_GroupParameters_EC<ECP>(CryptoPP::ASN1::secp256r1());

	const ECGroup::ECP ECGroup::EC_CURVE = EC_PARAMS.GetCurve();

	const ECGroup::Point ECGroup::EC_BASE_POINT = EC_PARAMS.GetSubgroupGenerator();

	const ECGroup::Self ECGroup::GENERATOR = Self(EC_BASE_POINT);

	const ECGroup::Self ECGroup::IDENTITY = Self(true); // isIdentity=true

	const Distribution<BigInt> ECGroup::DIST = Random<BigInt>::get_dist_below(order());

	GroupOrder ECGroup::order()
	{
		return EC_PARAMS.GetSubgroupOrder();
	}

	ECGroup::Self ECGroup::generator()
	{
		return GENERATOR;
	}

	ECGroup::Self ECGroup::identity()
	{
		return IDENTITY;
	}

	ECGroup::ECGroup(const BigInt& x, const BigInt& y)
		: _point(x, y), _isIdentity(false) { }

	ECGroup::Self ECGroup::from_scalar(const BigInt& scalar)
	{
		if (scalar.IsZero())
			return IDENTITY;
		return Self(EC_CURVE.Multiply(scalar, EC_BASE_POINT));
	}

	ECGroup::Self ECGroup::sample()
	{
		return from_scalar(DIST());
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
			return IDENTITY;
		BigInt p = EC_CURVE.GetField().GetModulus();
		BigInt negY = (p - this->_point.y) % p;
		return Self(this->_point.x, negY);
	}

	ECGroup::Self ECGroup::operator*(const Self& other) const
	{
		if (this->is_identity())
			return other;
		if (other.is_identity())
			return *this;
		return Self(EC_CURVE.Add(this->_point, other._point));
	}

	ECGroup::Self& ECGroup::operator*=(const Self& other)
	{
		if (other.is_identity())
			return *this;
		if (this->is_identity())
			return *this = other;
		this->_point = EC_CURVE.Add(this->_point, other._point);
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

	std::ostream& operator<<(std::ostream& os, const ECGroup& elem)
	{
		if (elem.is_identity())
			return os << "ECGroup(IDENTITY)";
		return os << "ECGroup(" << elem._point.x << ", " << elem._point.y << ")";
	}

	ECGroup::ECGroup(bool isIdentity) : _isIdentity(isIdentity) { }

	ECGroup::ECGroup(const Point& point) : _point(point), _isIdentity(false) { }
}
