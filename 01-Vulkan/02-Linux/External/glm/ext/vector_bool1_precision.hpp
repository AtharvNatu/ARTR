/// @ref ext_vector_bool1_precision
/// @file glm/ext/vector_bool1_precision.hpp
///
/// @defgroup ext_vector_bool1_precision GLM_EXT_vector_bool1_precision
/// @ingroup ext
///
/// Exposes highp_bvec1, mediump_bvec1 and lowp_bvec1 types.
///
/// Include <glm/ext/vector_bool1_precision.hpp> to use the features of this extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_EXT_vector_bool1_precision extension included")
#endif

namespace glm
{
	/// @addtogroup ext_vector_bool1_precision
	/// @{

	/// 1 component vector of Bool values.
	typedef vec<1, Bool, highp>			highp_bvec1;

	/// 1 component vector of Bool values.
	typedef vec<1, Bool, mediump>		mediump_bvec1;

	/// 1 component vector of Bool values.
	typedef vec<1, Bool, lowp>			lowp_bvec1;

	/// @}
}//namespace glm
