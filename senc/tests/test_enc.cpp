/*********************************************************************
 * \file   test_enc.cpp
 * \brief  Contains tests for encryption utilities.
 * 
 * \author aviad1b
 * \date   November 2025, Heshvan 5786
 *********************************************************************/

#include <gtest/gtest.h>

#include "../utils/enc/HybridElGamal2L.hpp"
#include "../utils/enc/ECHKDF2L.hpp"
#include "../utils/enc/AES1L.hpp"
#include "../utils/bytes.hpp"

using senc::utils::enc::HybridElGamal2L;
using senc::utils::enc::ECHKDF2L;
using senc::utils::enc::AES1L;
using senc::utils::ECGroup;
using senc::utils::Buffer;

struct EncDecTest : testing::Test, testing::WithParamInterface<Buffer> { };

TEST_P(EncDecTest, AES)
{
	AES1L schema;
	const auto key = schema.keygen();
	const Buffer& data = GetParam();
	const auto encrypted = schema.encrypt(data, key);
	const auto decrypted = schema.decrypt(encrypted, key);
	EXPECT_EQ(data, decrypted);
}

INSTANTIATE_TEST_CASE_P(AES, EncDecTest, testing::Values(
	Buffer({ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }),
	Buffer({ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 }),
	Buffer({ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
));

TEST_P(EncDecTest, HybridElGamal)
{
	HybridElGamal2L<ECGroup, AES1L, ECHKDF2L> schema;
	const auto [pubKey1, privKey1] = schema.keygen();
	const auto [pubKey2, privKey2] = schema.keygen();
	const Buffer& data = GetParam();
	const auto encrypted = schema.encrypt(data, pubKey1, pubKey2);
	const auto decrypted = schema.decrypt(encrypted, privKey1, privKey2);
	EXPECT_EQ(data, decrypted);
}

INSTANTIATE_TEST_CASE_P(HybridElGamal, EncDecTest, testing::Values(
	Buffer({ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }),
	Buffer({ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 }),
	Buffer({ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
));
