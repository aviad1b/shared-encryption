#include "AES1L.hpp"

#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>

namespace senc::utils::enc
{
	AES1L::Key AES1L::keygen() const
	{
		CryptoPP::SecByteBlock res(CryptoPP::AES::DEFAULT_KEYLENGTH);
		_prng.GenerateBlock(res, res.size());
		return res;
	}

	AES1L::Ciphertext AES1L::encrypt(const Plaintext& plaintext, const Key& key)
	{
		CryptoPP::SecByteBlock cipherIV(CryptoPP::AES::BLOCKSIZE);
		_prng.GenerateBlock(cipherIV, cipherIV.size());		

		CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
		encryptor.SetKeyWithIV(key, key.size(), cipherIV);

		// write binary output into cipherData:
		std::vector<CryptoPP::byte> cipherData;
		CryptoPP::ArraySource(
			plaintext.data(), plaintext.size(),
			true,
			new CryptoPP::StreamTransformationFilter(
				encryptor,
				new CryptoPP::VectorSink(cipherData)
			)
		);
	}
}
