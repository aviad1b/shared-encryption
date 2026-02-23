/*********************************************************************
 * \file   Value_impl.hpp
 * \brief  Implementation of `Value` class.
 * 
 * \author aviad1b
 * \date   February 2026, Adar 5786
 *********************************************************************/

#include "Value.hpp"

namespace senc::clientapi
{
	template <std::move_constructible T>
	inline Handle* Value<T>::new_instance(T&& value) noexcept
	{
		try { return new Self(true, std::move(value)); }
		catch (const std::bad_alloc&) { return Error::ALLOCATION; }
		catch (const std::exception& e) { return Error::new_instance(e.what()); }
	}

	template <std::move_constructible T>
	template <typename... Args>
	requires std::constructible_from<T, Args...>
	inline Handle* senc::clientapi::Value<T>::new_instance(Args&&... args) noexcept
	{
		return new_instance(T(std::forward<Args>(args)...));
	}

	template <std::move_constructible T>
	inline Handle* Value<T>::ret_new(utils::Callable<T> auto&& f) noexcept
	{
		try { return new_instance(f()); }
		catch (const std::bad_alloc&) { return Error::ALLOCATION; }
		catch (const std::exception& e) { return Error::new_instance(e.what()); }
		return Error::UNKNOWN;
	}

	template <std::move_constructible T>
	inline bool Value<T>::has_error() const noexcept
	{
		return false;
	}

	template <std::move_constructible T>
	inline T& Value<T>::get() noexcept
	{
		return _value;
	}

	template <std::move_constructible T>
	const T& Value<T>::get() const noexcept
	{
		return _value;
	}

	template <std::move_constructible T>
	inline Value<T>::Value(bool isAllocated, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
		: Base(isAllocated), _value(std::move(value)) { }
}
