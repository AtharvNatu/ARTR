/// @ref gtx_scalar_relational

namespace glm
{
	template<typename T>
	GLM_FUNC_QUALIFIER Bool lessThan
	(
		T const& x,
		T const& y
	)
	{
		return x < y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER Bool lessThanEqual
	(
		T const& x,
		T const& y
	)
	{
		return x <= y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER Bool greaterThan
	(
		T const& x,
		T const& y
	)
	{
		return x > y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER Bool greaterThanEqual
	(
		T const& x,
		T const& y
	)
	{
		return x >= y;
	}

	template<typename T>
	GLM_FUNC_QUALIFIER Bool equal
	(
		T const& x,
		T const& y
	)
	{
		return detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::call(x, y);
	}

	template<typename T>
	GLM_FUNC_QUALIFIER Bool notEqual
	(
		T const& x,
		T const& y
	)
	{
		return !detail::compute_equal<T, std::numeric_limits<T>::is_iec559>::call(x, y);
	}

	GLM_FUNC_QUALIFIER Bool any
	(
		Bool const& x
	)
	{
		return x;
	}

	GLM_FUNC_QUALIFIER Bool all
	(
		Bool const& x
	)
	{
		return x;
	}

	GLM_FUNC_QUALIFIER Bool not_
	(
		Bool const& x
	)
	{
		return !x;
	}
}//namespace glm
