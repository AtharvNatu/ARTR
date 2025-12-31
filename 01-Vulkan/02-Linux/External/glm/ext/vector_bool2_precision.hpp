/// @ref core
/// @file glm/ext/vector_bool2_precision.hpp

#pragma once
#include "../detail/type_vec2.hpp"

namespace glm
{
	/// @addtogroup core_vector_precision
	/// @{

	/// 2 components vector of high qualifier Bool numbers.
	///
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.1.5 Vectors</a>
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.7.2 Precision Qualifier</a>
	typedef vec<2, Bool, highp>		highp_bvec2;

	/// 2 components vector of medium qualifier Bool numbers.
	///
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.1.5 Vectors</a>
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.7.2 Precision Qualifier</a>
	typedef vec<2, Bool, mediump>	mediump_bvec2;

	/// 2 components vector of low qualifier Bool numbers.
	///
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.1.5 Vectors</a>
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.7.2 Precision Qualifier</a>
	typedef vec<2, Bool, lowp>		lowp_bvec2;

	/// @}
}//namespace glm
