#pragma once

#include <exception>
#include <string>

namespace senc::utils
{
	class Exception : public std::exception
	{
	public:
		using Self = Exception;

		Exception(const std::string& msg);

		Exception(std::string&& msg);

		Exception(const Self&) = default;

		Self& operator=(const Self&) = default;

		Exception(Self&&) = default;

		Self& operator=(Self&&) = default;

		const char* what() const noexcept override;

	private:
		std::string _msg;
	};
}
