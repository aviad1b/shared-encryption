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

namespace senc::client
{
	/**
	 * @concept senc::client::NumInputable
	 * @brief Looks for a numeric-representing typename which can be inputted by `input_num`.
	 * @note `input_num` Casts input number from `long long`.
	 * @tparam Self Examined typename.
	 */
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
		) ||
		std::same_as<Self, utils::BigInt>;

	namespace sfinae
	{
		template <typename T>
		struct num_inputable_int { using type = T; };

		template <utils::ModIntType T>
		struct num_inputable_int<T> { using type = typename T::Int; };

		template <>
		struct num_inputable_int<utils::BigInt> { using type = long long; };
	}

	/**
	 * @typedef senc::client::NumInputableInt
	 * @brief Fundamental integer type used for numeric input for a given result representative type.
	 * @tparam T Number-representing type which will be constructed from `NumInputableInt<T>`.
	 */
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

	/**
	 * @brief Minimum acceptable value for numeric input.
	 * @tparam T Numeric input type.
	 */
	template <NumInputable T>
	constexpr NumInputableInt<T> NUM_INPUTABLE_MIN = sfinae::num_inputable_min<T>::value;

	/**
	 * @brief Maximum acceptable value for numeric input.
	 * @tparam T Numeric input type.
	 */
	template <NumInputable T>
	constexpr NumInputableInt<T> NUM_INPUTABLE_MAX = sfinae::num_inputable_max<T>::value;

	/**
	 * @brief Gets string input.
	 * @return String input.
	 */
	std::string input();

	/**
	 * @brief Gets string input.
	 * @param msg Message to print before input.
	 * @return String input.
	 */
	std::string input(const std::string& msg);

	/**
	 * @brief Gets yes/no input.
	 * @return `true` for yes ("y"/"Y"), `false` for no ("n"/"N").
	 */
	bool input_yesno();

	/**
	 * @brief Gets yes/no input.
	 * @param msg Message to print before input.
	 * @return `true` for yes ("y"/"Y"), `false` for no ("n"/"N").
	 */
	bool input_yesno(const std::string& msg);

	/**
	 * @brief Gets vector of string inputs, until reaches an empty input.
	 * @tparam elemInput Function used for inputting each element.
	 * @return Vector of inputs.
	 */
	template <std::string(*elemInput)() = input>
	std::vector<std::string> input_vec();

	/**
	 * @brief Gets vector of string inputs, until reaches an empty input.
	 * @tparam elemInput Function used for inputting each element.
	 * @param msg Message to print before input.
	 * @return Vector of inputs.
	 */
	template <std::string(*elemInput)() = input>
	std::vector<std::string> input_vec(const std::string& msg);

	/**
	 * @brief Gets vector of inputs, until reaches an empty input.
	 * @tparam T Input element type.
	 * @tparam elemInput Function used for inputting each element, returning `nullopt` for empty input.
	 * @return Vector of inputs.
	 */
	template <typename T, std::optional<T>(*elemInput)()>
	std::vector<T> input_vec();

	/**
	 * @brief Gets vector of inputs, until reaches an empty input.
	 * @tparam T Input element type.
	 * @tparam elemInput Function used for inputting each element, returning `nullopt` for empty input.
	 * @param msg Message to print before input.
	 * @return Vector of inputs.
	 */
	template <typename T, std::optional<T>(*elemInput)()>
	std::vector<T> input_vec(const std::string& msg);

	/**
	 * @brief Gets numeric input.
	 * @tparam T Number-representing type to use for input, must satisfy `NumInputable`.
	 * @tparam allowEmpty Whether or not should allow empty input.
	 * @tparam MIN Minimum bound for input.
	 * @tparam MAX Maximum bound for input.
	 * @return Numeric input, or `std::nullopt` for empty input.
	 */
	template <
		NumInputable T,
		bool allowEmpty = false,
		NumInputableInt<T> MIN = NUM_INPUTABLE_MIN<T>,
		NumInputableInt<T> MAX = NUM_INPUTABLE_MAX<T>
	>
	std::conditional_t<allowEmpty, std::optional<T>, T> input_num();

	/**
	 * @brief Gets numeric input.
	 * @tparam T Number-representing type to use for input, must satisfy `NumInputable`.
	 * @tparam allowEmpty Whether or not should allow empty input.
	 * @tparam MIN Minimum bound for input.
	 * @tparam MAX Maximum bound for input.
	 * @param msg Message to print before input.
	 * @return Numeric input, or `std::nullopt` for empty input.
	 */
	template <
		NumInputable T,
		bool allowEmpty = false,
		NumInputableInt<T> MIN = NUM_INPUTABLE_MIN<T>,
		NumInputableInt<T> MAX = NUM_INPUTABLE_MAX<T>
	>
	std::conditional_t<allowEmpty, std::optional<T>, T> input_num(const std::string& msg);

	/**
	 * @brief Gets multiple numeric inputs (until empty input).
	 * @tparam T Number-representing type to use for input, must satisfy `NumInputable`.
	 * @tparam MIN Minimum bound for input.
	 * @tparam MAX Maximum bound for input.
	 * @return Vector of numeric inputs.
	 */
	template <
		NumInputable T,
		NumInputableInt<T> MIN = NUM_INPUTABLE_MIN<T>,
		NumInputableInt<T> MAX = NUM_INPUTABLE_MAX<T>
	>
	std::vector<T> input_num_vec();

	/**
	 * @brief Gets multiple numeric inputs (until empty input).
	 * @tparam T Number-representing type to use for input, must satisfy `NumInputable`.
	 * @tparam MIN Minimum bound for input.
	 * @tparam MAX Maximum bound for input.
	 * @param msg Message to print before input.
	 * @return Vector of numeric inputs.
	 */
	template <
		NumInputable T,
		NumInputableInt<T> MIN = NUM_INPUTABLE_MIN<T>,
		NumInputableInt<T> MAX = NUM_INPUTABLE_MAX<T>
	>
	std::vector<T> input_num_vec(const std::string& msg);

	/**
	 * @brief Gets UUID input.
	 * @return UUID input.
	 */
	utils::UUID input_uuid();

	/**
	 * @brief Gets UUID input.
	 * @param msg Message to print before input.
	 * @return UUID input.
	 */
	utils::UUID input_uuid(const std::string& msg);

	/**
	 * @brief Gets username input.
	 * @return Username input.
	 */
	std::string input_username();

	/**
	 * @brief Gets username input.
	 * @param msg Message to print before input.
	 * @return Username input.
	 */
	std::string input_username(const std::string& msg);

	/**
	 * @brief Gets multiple username inputs (until empty input).
	 * @return Username inputs vector.
	 */
	std::vector<std::string> input_usernames();

	/**
	 * @brief Gets multiple username inputs (until empty input).
	 * @param msg Message to print before input.
	 * @return Username inputs vector.
	 */
	std::vector<std::string> input_usernames(const std::string& msg);

	/**
	 * @brief Gets threahold number input.
	 * @return Threshold number input.
	 */
	member_count_t input_threshold();

	/**
	 * @brief Gets threahold number input.
	 * @param msg Message to print before input.
	 * @return Threshold number input.
	 */
	member_count_t input_threshold(const std::string& msg);

	/**
	 * @brief Gets userset ID input.
	 * @return Userset ID input.
	 */
	UserSetID input_userset_id();

	/**
	 * @brief Gets userset ID input.
	 * @param msg Message to print before input.
	 * @return Userset ID input.
	 */
	UserSetID input_userset_id(const std::string& msg);

	/**
	 * @brief Gets operation ID input.
	 * @return Operation ID input.
	 */
	OperationID input_operation_id();

	/**
	 * @brief Gets operation ID input.
	 * @param msg Message to print before input.
	 * @return Operation ID input.
	 */
	OperationID input_operation_id(const std::string& msg);

	/**
	 * @brief Gets public keys input.
	 * @return Public keys input.
	 */
	std::pair<PubKey, PubKey> input_pub_keys();

	/**
	 * @brief Gets public keys input.
	 * @param msg Message to print before input.
	 * @return Public keys input.
	 */
	std::pair<PubKey, PubKey> input_pub_keys(const std::string& msg);

	/**
	 * @brief Gets multiple private key shard ID inputs (until empty input).
	 * @return Private key shard IDs input.
	 */
	std::vector<PrivKeyShardID> input_priv_key_shard_ids();

	/**
	 * @brief Gets multiple private key shard ID input (until empty input).
	 * @param msg Message to print before input.
	 * @return Private key shard IDs input.
	 */
	std::vector<PrivKeyShardID> input_priv_key_shard_ids(const std::string& msg);

	/**
	 * @brief Gets private key shard input.
	 * @return Private key shard input.
	 */
	PrivKeyShard input_priv_key_shard();

	/**
	 * @brief Gets private key shard input.
	 * @param msg Message to print before input.
	 * @return Private key shard input.
	 */
	PrivKeyShard input_priv_key_shard(const std::string& msg);

	/**
	 * @brief Gets ciphertext input.
	 * @return Ciphertext input.
	 */
	Ciphertext input_ciphertext();

	/**
	 * @brief Gets ciphertext input.
	 * @param msg Message to print before input.
	 * @return Ciphertext input.
	 */
	Ciphertext input_ciphertext(const std::string& msg);

	/**
	 * @brief Gets decryption part input.
	 * @tparam allowEmpty Whether or not should allow empty input.
	 * @return Decryption part input (or `std::nullopt` for empty input).
	 */
	template <bool allowEmpty = false>
	std::conditional_t<allowEmpty, std::optional<DecryptionPart>, DecryptionPart> input_decryption_part();

	/**
	 * @brief Gets decryption part input.
	 * @tparam allowEmpty Whether or not should allow empty input.
	 * @param msg Message to print before input.
	 * @return Decryption part input (or `std::nullopt` for empty input).
	 */
	template <bool allowEmpty = false>
	std::conditional_t<allowEmpty, std::optional<DecryptionPart>, DecryptionPart>
		input_decryption_part(const std::string& msg);

	/**
	 * @brief Gets multiple decryption part inputs.
	 * @return Decryption part inputs vector.
	 */
	std::vector<DecryptionPart> input_decryption_parts();

	/**
	 * @brief Gets multiple decryption part inputs.
	 * @param msg Message to print before input.
	 * @return Decryption part inputs vector.
	 */
	std::vector<DecryptionPart> input_decryption_parts(const std::string& msg);
}

#include "input_impl.hpp"
