/*********************************************************************
 * \file   input_impl.hpp
 * \brief  Template implementation of client input utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "input.hpp"

#include <iostream>

namespace senc::client
{
	template <std::string(*elemInput)()>
	inline std::vector<std::string> input_vec()
	{
		std::vector<std::string> res;
		std::string curr;

		while (!(curr = elemInput()).empty())
			res.push_back(curr);

		return res;
	}

	template <std::string(*elemInput)()>
	inline std::vector<std::string> input_vec(const std::string& msg)
	{
		std::cout << msg;
		return input_vec<elemInput>();
	}

	template <typename T, std::optional<T>(*elemInput)()>
	inline std::vector<T> input_vec()
	{
		std::vector<T> res;
		std::optional<T> curr;

		while ((curr = elemInput()).has_value())
			res.push_back(*curr);

		return res;
	}

	template <typename T, std::optional<T>(*elemInput)()>
	inline std::vector<T> input_vec(const std::string& msg)
	{
		std::cout << msg;
		return input_vec<T, elemInput>();
	}

	template <
		NumInputable T,
		bool allowEmpty,
		NumInputableInt<T> MIN,
		NumInputableInt<T> MAX
	>
	inline std::conditional_t<allowEmpty, std::optional<T>, T> input_num()
	{
		using Int = NumInputableInt<T>;
		bool invalid = false;
		long long num = 0;

		do
		{
			invalid = false;
			std::string str = input();

			if constexpr (allowEmpty)
				if (str.empty())
					return std::nullopt;

			try { num = std::stoll(str); }
			catch (const std::exception&)
			{
				std::cout << "Bad input (should be number in range " << MIN << ".." << MAX << ")." << std::endl;
				std::cout << "Try again: ";
				invalid = true;
			}
		} while (invalid);

		return static_cast<Int>(num);
	}

	template <
		NumInputable T,
		bool allowEmpty,
		NumInputableInt<T> MIN,
		NumInputableInt<T> MAX
	>
	inline std::conditional_t<allowEmpty, std::optional<T>, T> input_num(const std::string& msg)
	{
		std::cout << msg;
		return input_num<T, allowEmpty, MIN, MAX>();
	}

	template <NumInputable T, NumInputableInt<T> MIN, NumInputableInt<T> MAX>
	std::vector<T> input_num_vec()
	{
		return input_vec<T, input_num<T, true, MIN, MAX>>();
	}

	template <NumInputable T, NumInputableInt<T> MIN, NumInputableInt<T> MAX>
	std::vector<T> input_num_vec(const std::string& msg)
	{
		return input_vec<T, input_num<T, true, MIN, MAX>>(msg);
	}

	template <bool allowEmpty>
	std::conditional_t<allowEmpty, std::optional<DecryptionPart>, DecryptionPart> input_decryption_part()
	{
		std::string str = input();
		if constexpr (allowEmpty)
			if (str.empty())
				return std::nullopt;
		return DecryptionPart::from_bytes(utils::bytes_from_base64(str));
	}

	template <bool allowEmpty>
	std::conditional_t<allowEmpty, std::optional<DecryptionPart>, DecryptionPart>
		input_decryption_part(const std::string& msg)
	{
		std::cout << msg;
		return input_decryption_part<allowEmpty>();
	}
}
