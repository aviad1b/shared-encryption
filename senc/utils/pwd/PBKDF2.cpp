/*********************************************************************
 * \file   PBKDF2.cpp
 * \brief  Implementation of PBKDF2 class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "PBKDF2.hpp"

namespace senc::utils::pwd
{
    PBKDF2::PBKDF2(std::size_t iterations) : _iterations(iterations) { }

    PBKDF2::Salt PBKDF2::generate_salt()
    {
        Salt salt{};
        _saltRng.GenerateBlock(salt.data(), salt.size());
        return salt;
    }

    PBKDF2::Output PBKDF2::hash(const Password& password, const Salt& salt)
    {
        Output res{};
        _pbkdf2.DeriveKey(
            res.data(), res.size(),
            0, // `purpose` is unused
            reinterpret_cast<const byte*>(password.data()),
            password.size() * sizeof(Password::value_type),
            salt.data(), salt.size(),
            _iterations
        );
        return res;
    }
}
