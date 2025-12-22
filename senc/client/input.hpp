/*********************************************************************
 * \file   input.hpp
 * \brief  Header of client input utilities.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#pragma once

#include "../common/packets.hpp"
#include "../utils/ModInt.hpp"

namespace senc
{
	template <typename Self>
	concept NumInputable = 
		(
			std::integral<Self> &&
			(std::numeric_limits<Self>::min() >= std::numeric_limits<long long>::min()) &&
			(std::numeric_limits<Self>::max() <= std::numeric_limits<long long>::max())
		) ||
		(
			utils::ModIntType<Self> &&
			(Self::modulus() - 1 <= std::numeric_limits<long long>::max())
		);

	namespace sfinae
	{
		template <typename T>
		struct num_inputable_int { using type = T; };

		template <utils::ModIntType T>
		struct num_inputable_int<T> { using type = typename T::Int; };
	}

	template <NumInputable T>
	using NumInputableInt = typename sfinae::num_inputable_int<T>::type;

	namespace sfinae
	{
		template <typename T>
		struct num_inputable_min : std::integral_constant<
			NumInputableInt<T>,
			std::numeric_limits<NumInputableInt<T>>::min()
		> { };

		template <utils::ModIntType T>
		struct num_inputable_min<T> : std::integral_constant<
			NumInputableInt<T>,
			static_cast<NumInputableInt<T>>(0)
		> { };

		template <typename T>
		struct num_inputable_max : std::integral_constant<
			NumInputableInt<T>,
			std::numeric_limits<NumInputableInt<T>>::max()
		> { };

		template <utils::ModIntType T>
		struct num_inputable_max<T> : std::integral_constant<
			NumInputableInt<T>,
			T::modulus()
		>{ };
	}

	template <NumInputable T>
	constexpr NumInputableInt<T> NUM_INPUTABLE_MIN = sfinae::num_inputable_min<T>::value;

	template <NumInputable T>
	constexpr NumInputableInt<T> NUM_INPUTABLE_MAX = sfinae::num_inputable_max<T>::value;

	std::string input();
	std::string input(const std::string& msg);

	bool input_yesno();
	bool input_yesno(const std::string& msg);

	template <std::string(*elemInput)() = input>
	std::vector<std::string> input_vec();

	template <std::string(*elemInput)() = input>
	std::vector<std::string> input_vec(const std::string& msg);

	template <typename T, std::optional<T>(*elemInput)()>
	std::vector<T> input_vec();

	template <typename T, std::optional<T>(*elemInput)()>
	std::vector<T> input_vec(const std::string& msg);

	template <NumInputable T, bool allowEmpty = false>
	std::conditional_t<allowEmpty, std::optional<T>, T> input_num();

	template <NumInputable T, bool allowEmpty = false>
	std::conditional_t<allowEmpty, std::optional<T>, T> input_num(const std::string& msg);

	template <NumInputable T>
	std::vector<T> input_num_vec();

	template <NumInputable T>
	std::vector<T> input_num_vec(const std::string& msg);

	utils::UUID input_uuid();
	utils::UUID input_uuid(const std::string& msg);

	std::string input_username();
	std::string input_username(const std::string& msg);

	std::vector<std::string> input_usernames();
	std::vector<std::string> input_usernames(const std::string& msg);

	member_count_t input_threshold();
	member_count_t input_threshold(const std::string& msg);

	UserSetID input_userset_id();
	UserSetID input_userset_id(const std::string& msg);

	OperationID input_operation_id();
	OperationID input_operation_id(const std::string& msg);

	std::vector<PrivKeyShardID> input_priv_key_shard_ids();
	std::vector<PrivKeyShardID> input_priv_key_shard_ids(const std::string& msg);

	PrivKeyShard input_priv_key_shard();
	PrivKeyShard input_priv_key_shard(const std::string& msg);

	Ciphertext input_ciphertext();

	template <bool allowEmpty = false>
	std::conditional_t<allowEmpty, std::optional<DecryptionPart>, DecryptionPart> input_decryption_part();

	template <bool allowEmpty = false>
	std::conditional_t<allowEmpty, std::optional<DecryptionPart>, DecryptionPart>
		input_decryption_part(const std::string& msg);

	std::vector<DecryptionPart> input_decryption_parts();
	std::vector<DecryptionPart> input_decryption_parts(const std::string& msg);
}

#include "input_impl.hpp"
