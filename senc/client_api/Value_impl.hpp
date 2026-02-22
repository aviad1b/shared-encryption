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
	template <std::movable T>
	inline Handle* Value<T>::new_instance(T&& value) noexcept
	{
		try { return new Self(true, std::move(value)); }
		catch (const std::bad_alloc&) { return Error::ALLOCATION; }
		catch (const std::exception& e) { return Error::new_instance(e.what()); }
	}

	template <std::movable T>
	inline bool Value<T>::has_error() const
	{
		return false;
	}

	template <std::movable T>
	inline T& Value<T>::get() noexcept
	{
		return _value;
	}

	template <std::movable T>
	const T& Value<T>::get() const noexcept
	{
		return _value;
	}

	template <std::movable T>
	inline Value<T>::Value(bool isAllocated, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
		: Base(isAllocated), _value(std::move(value)) { }
}
