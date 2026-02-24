/*********************************************************************
 * \file   PBKDF2_impl.hpp
 * \brief  Implementation of PBKDF2 class.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#include "PBKDF2.hpp"

namespace senc::utils::pwd
{
    template <std::size_t outputSize, std::size_t saltSize>
    PBKDF2<outputSize, saltSize>::PBKDF2(std::size_t iterations) : _iterations(iterations) { }

    template <std::size_t outputSize, std::size_t saltSize>
    PBKDF2<outputSize, saltSize>::Salt PBKDF2<outputSize, saltSize>::generate_salt()
    {
        Salt salt{};
        _saltRng.GenerateBlock(salt.data(), salt.size());
        return salt;
    }

    template <std::size_t outputSize, std::size_t saltSize>
    PBKDF2<outputSize, saltSize>::Output PBKDF2<outputSize, saltSize>::hash(
        const Password& password, const Salt& salt)
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
