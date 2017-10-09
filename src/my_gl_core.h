/*!
\author Borja Portugal Martin
*/

#pragma once

#include "gl_core\gl_core_4_2.hpp"

namespace my_gl_core
{
	unsigned get_opengl_mayor_v();
	unsigned get_opengl_minor_v();

	void break_on_error(bool b);
	bool is_break_on_error_enabled();
};

#if _DEBUG
namespace my_gl_core
{
	namespace impl
	{
		bool CheckOpenGLError(GLenum error);
	}
}

// TODO(Borja): Pass the function and line where the erro happened.
#define CheckOGLError(...)	\
	do { if (my_gl_core::impl::CheckOpenGLError(gl::GetError())) DebugBreak(); } while(0)

#else
#	define CheckOGLError(...)	do {} while (0)
#endif


