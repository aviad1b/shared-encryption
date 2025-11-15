#include "AES1L.hpp"

namespace senc::utils::enc
{
	AES1L::Key AES1L::keygen() const
	{
		CryptoPP::SecByteBlock res(CryptoPP::AES::DEFAULT_KEYLENGTH);
		_prng.GenerateBlock(res, res.size());
		return res;
	}
}
