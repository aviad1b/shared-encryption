/*********************************************************************
 * \file   ip.hpp
 * \brief  Header of IP-related utilities.
 * 
 * \author aviad1b
 * \date   January 2026, Teveth 5786
 *********************************************************************/

#pragma once

#include "env.hpp"
#ifdef SENC_WINDOWS
#include "winapi_patch.hpp"
#include <ws2ipdef.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <concepts>

#include "Exception.hpp"
#include "port.hpp"

namespace senc::utils
{
	/**
	 * @class senc::utils::IPException
	 * @brief Type of exceptions thrown on IP-related errors.
	 */
	class IPException : public Exception
	{
	public:
		using Self = IPException;
		using Base = Exception;

		IPException(const std::string& msg) : Base(msg) { }

		IPException(std::string&& msg) : Base(std::move(msg)) { }

		IPException(const std::string& msg, const std::string& info) : Base(msg, info) { }
		
		IPException(std::string&& msg, const std::string& info) : Base(std::move(msg), info) { }

		IPException(const Self&) = default;

		Self& operator=(const Self&) = default;

		IPException(Self&&) = default;

		Self& operator=(Self&&) = default;
	};

	/**
	 * @concept senc::utils::IPType
	 * @brief Looks for a typename that can be used as an IP address (e.g. IPv4, IPv6).
	 * @tparam Self Examined typename.
	 */
	template <typename Self>
	concept IPType = std::copyable<Self> &&
		std::equality_comparable<Self> &&
		std::constructible_from<Self, typename Self::Underlying> &&
		requires(
			const Self self, typename Self::UnderlyingSockAddr* out,
			const typename Self::UnderlyingSockAddr underlyingSockAddr
		)
		{
			{ Self::UNDERLYING_ADDRESS_FAMILY } -> std::convertible_to<int>;
			{ Self::any() } -> std::convertible_to<const Self&>;
			{ Self::loopback() } -> std::convertible_to<const Self&>;
			{ self.as_str() } noexcept -> std::convertible_to<const std::string&>;
			{ self.init_underlying(out, std::declval<Port>()) } noexcept;
			{ Self::from_underlying_sock_addr(underlyingSockAddr) } -> std::same_as<std::tuple<Self, Port>>;
		};

	/**
	 * @class senc::utils::IPv4
	 * @brief Represents an IPv4 address.
	 */
	class IPv4
	{
	public:
		using Self = IPv4;
		using Underlying = struct in_addr;
		using UnderlyingSockAddr = struct sockaddr_in;
		static constexpr int UNDERLYING_ADDRESS_FAMILY = AF_INET;

		/**
		 * @brief Gets IP used for binding socket to any address.
		 */
		static const Self& any();

		/**
		 * @brief Gets IP used for loopback.
		 */
		static const Self& loopback();

		/**
		 * @brief Constructs an IPv4 address from underlying struct.
		 * @param underlying Underlying struct instance.
		 */
		explicit IPv4(const Underlying& underlying);

		/**
		 * @brief Gets IPv4 address and port from underlying sockaddr struct.
		 */
		static std::tuple<Self, Port> from_underlying_sock_addr(
			const UnderlyingSockAddr& underlyingSockAddr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address.
		 * @throw senc::utils::IPException If `addr` is not a valid IPv4 address.
		 */
		IPv4(const char* addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address.
		 * @throw senc::utils::IPException If `addr` is not a valid IPv4 address.
		 */
		IPv4(const std::string& addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address (moved).
		 * @throw senc::utils::IPException If `addr` is not a valid IPv4 address.
		 */
		IPv4(std::string&& addr);

		/**
		 * @brief Copy constructor of IPv4 address.
		 */
		IPv4(const Self&) = default;

		/**
		 * @brief Copy assignment operator of IPv4 address.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of IPv4 address.
		 */
		IPv4(Self&&) = default;

		/**
		 * @brief Move assignment operator of IPv4 Address.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Compares the IPv4 address to another.
		 * @param other Other IPv4 address to compare to.
		 * @param `true` if `*this` is the same address as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Gets string representation of IPv4 address.
		 * @return String representation of IPv4 address.
		 */
		const std::string& as_str() const noexcept;

		/**
		 * @brief Initializes underlying library's struct based on IPv4 address and port.
		 * @param out Address of underlying library's struct to initialize.
		 * @param port Port to initialize based on.
		 */
		void init_underlying(UnderlyingSockAddr* out, Port port) const noexcept;

	private:
		Underlying _addr{};
		std::string _addrStr;
	};
	static_assert(IPType<IPv4>);

	/**
	 * @class senc::utils::IPv6
	 * @brief Represents an IPv6 address.
	 */
	class IPv6
	{
	public:
		using Self = IPv6;
		using Underlying = struct in6_addr;
		using UnderlyingSockAddr = struct sockaddr_in6;
		static constexpr int UNDERLYING_ADDRESS_FAMILY = AF_INET6;

		/**
		 * @brief Gets IP used for binding socket to any address.
		 */
		static const Self& any();

		/**
		 * @brief Gets IP used for loopback.
		 */
		static const Self& loopback();

		/**
		 * @brief Constructs an IPv6 address from underlying struct.
		 * @param underlying Underlying struct instance.
		 */
		explicit IPv6(const Underlying& underlying);

		/**
		 * @brief Gets IPv6 address and port from underlying sockaddr struct.
		 */
		static std::tuple<Self, Port> from_underlying_sock_addr(
			const UnderlyingSockAddr& underlyingSockAddr);

		/**
		 * @brief Constructs an IPv6 address from string representation.
		 * @param addr String representation of IPv6 address.
		 * @throw senc::utils::IPException If `addr` is not a valid IPv6 address.
		 */
		IPv6(const char* addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address.
		 * @throw senc::utils::IPException If `addr` is not a valid IPv6 address.
		 */
		IPv6(const std::string& addr);

		/**
		 * @brief Constructs an IPv4 address from string representation.
		 * @param addr String representation of IPv4 address (moved).
		 * @throw senc::utils::IPException If `addr` is not a valid IPv6 address.
		 */
		IPv6(std::string&& addr);

		/**
		 * @brief Copy constructor of IPv4 address.
		 */
		IPv6(const Self&) = default;

		/**
		 * @brief Copy assignment operator of IPv4 address.
		 */
		Self& operator=(const Self&) = default;

		/**
		 * @brief Move constructor of IPv4 address.
		 */
		IPv6(Self&&) = default;

		/**
		 * @brief Move assignment operator of IPv4 Address.
		 */
		Self& operator=(Self&&) = default;

		/**
		 * @brief Compares the IPv4 address to another.
		 * @param other Other IPv4 address to compare to.
		 * @param `true` if `*this` is the same address as `other`, otherwise `false`.
		 */
		bool operator==(const Self& other) const noexcept;

		/**
		 * @brief Gets string representation of IPv4 address.
		 * @return String representation of IPv4 address.
		 */
		const std::string& as_str() const noexcept;

		/**
		 * @brief Initializes underlying library's struct based on IPv4 address and port.
		 * @param out Address of underlying library's struct to initialize.
		 * @param port Port to initialize based on.
		 * @throw senc::utils::IPException On failure.
		 */
		void init_underlying(UnderlyingSockAddr* out, Port port) const noexcept;

	private:
		Underlying _addr{};
		std::string _addrStr;
	};
	static_assert(IPType<IPv6>);
}
