#pragma once

//#include "compute_common.hpp"
#include "setup.hpp"
#include <limits>

namespace glm{
namespace detail
{
	template <typename T, Bool isFloat>
	struct compute_equal
	{
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR static Bool call(T a, T b)
		{
			return a == b;
		}
	};
/*
	template <typename T>
	struct compute_equal<T, True>
	{
		GLM_FUNC_QUALIFIER GLM_CONSTEXPR static Bool call(T a, T b)
		{
			return detail::compute_abs<T, std::numeric_limits<T>::is_signed>::call(b - a) <= static_cast<T>(0);
			//return std::memcmp(&a, &b, sizeof(T)) == 0;
		}
	};
*/
}//namespace detail
}//namespace glm
