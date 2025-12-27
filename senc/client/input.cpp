/*********************************************************************
 * \file   input.cpp
 * \brief  Implementation of client input utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "input.hpp"

#include <iostream>

namespace senc::client
{
	std::string input()
	{
		std::string res;
		std::getline(std::cin, res);
		return res;
	}

	std::string input(const std::string& msg)
	{
		std::cout << msg;
		return input();
	}

	bool input_yesno()
	{
		std::string res = input();
		while (res != "y" && res != "Y" && res != "n" && res != "N")
			res = input("Bad input, try again: ");
		return (res == "y" || res == "Y");
	}

	bool input_yesno(const std::string& msg)
	{
		std::cout << msg;
		return input_yesno();
	}

	utils::UUID input_uuid()
	{
		while (true)
		{
			try { return utils::UUID(input()); }
			catch (const utils::UUIDException&) { std::cout << "Bad input, try again: "; }
		}
	}

	utils::UUID input_uuid(const std::string& msg)
	{
		std::cout << msg;
		return input_uuid();
	}

	std::string input_username()
	{
		return input();
	}

	std::string input_username(const std::string& msg)
	{
		std::cout << msg;
		return input_username();
	}

	std::vector<std::string> input_usernames()
	{
		return input_vec<input_username>();
	}

	std::vector<std::string> input_usernames(const std::string& msg)
	{
		std::cout << msg;
		return input_usernames();
	}

	member_count_t input_threshold()
	{
		return input_num<member_count_t>();
	}

	member_count_t input_threshold(const std::string& msg)
	{
		std::cout << msg;
		return input_threshold();
	}

	UserSetID input_userset_id()
	{
		return input_uuid();
	}

	UserSetID input_userset_id(const std::string& msg)
	{
		std::cout << msg;
		return input_userset_id();
	}

	OperationID input_operation_id()
	{
		return input_uuid();
	}

	OperationID input_operation_id(const std::string& msg)
	{
		std::cout << msg;
		return input_operation_id();
	}

	std::pair<PubKey, PubKey> input_pub_keys()
	{
		auto pubKey1 = PubKey::from_bytes(utils::bytes_from_base64(input()));
		auto pubKey2 = PubKey::from_bytes(utils::bytes_from_base64(input()));
		return { std::move(pubKey1), std::move(pubKey2) };
	}

	std::pair<PubKey, PubKey> input_pub_keys(const std::string& msg)
	{
		std::cout << msg;
		return input_pub_keys();
	}

	std::vector<PrivKeyShardID> input_priv_key_shard_ids()
	{
		return input_vec<PrivKeyShardID, input_num<PrivKeyShardID, true>>();
	}

	std::vector<PrivKeyShardID> input_priv_key_shard_ids(const std::string& msg)
	{
		std::cout << msg;
		return input_priv_key_shard_ids();
	}

	PrivKeyShard input_priv_key_shard()
	{
		bool valid = false;
		PrivKeyShard res{};
		do
		{
			std::string str = input();
			const auto commaIndex = str.find(',');
			if (str.empty() || str[0] != '(' || str[str.length() - 1] != ')' || commaIndex == std::string::npos)
			{
				std::cout << "Invalid input, try again: ";
				continue;
			}

			std::string idStr = str.substr(0, commaIndex);
			std::string valStr = str.substr(commaIndex + 1);

			try
			{
				res.first = PrivKeyShardID(idStr.c_str());
				res.second = utils::BigInt(valStr.c_str());
			}
			catch (const std::exception&)
			{
				std::cout << "Invalid input, try again: ";
				continue;
			}

			valid = true; // if reached here, input is valid
		} while (!valid);

		return res;
	}

	PrivKeyShard input_priv_key_shard(const std::string& msg)
	{
		std::cout << msg;
		return input_priv_key_shard();
	}

	Ciphertext input_ciphertext()
	{
		auto c1 = utils::ECGroup::from_bytes(utils::bytes_from_base64(input()));
		auto c2 = utils::ECGroup::from_bytes(utils::bytes_from_base64(input()));
		auto c3aBuffer = utils::bytes_from_base64(input());
		auto c3b = utils::bytes_from_base64(input());

		CryptoPP::SecByteBlock c3a(c3aBuffer.data(), c3aBuffer.size());
		utils::enc::AES1L::Ciphertext c3{ c3a, c3b };

		return { std::move(c1), std::move(c2), std::move(c3) };
	}

	Ciphertext input_ciphertext(const std::string& msg)
	{
		std::cout << msg;
		return input_ciphertext();
	}

	std::vector<DecryptionPart> input_decryption_parts()
	{
		return input_vec<DecryptionPart, input_decryption_part<true>>();
	}

	std::vector<DecryptionPart> input_decryption_parts(const std::string& msg)
	{
		std::cout << msg;
		return input_decryption_parts();
	}
}
