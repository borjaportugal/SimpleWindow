/*!
\author Borja Portugal Martin
*/

#include "my_gl_core.h"

namespace my_gl_core
{
	unsigned get_opengl_mayor_v()
	{
		return 4;
	}
	unsigned get_opengl_minor_v()
	{
		return 2;
	}
	
	namespace impl
	{
		/// \brief	Store if we need to call DebugBreak when an OpenGL error occurs.
		struct BreakOnError 
		{ 
			static bool s_value; 
		};
		bool BreakOnError::s_value = false;
	}

	void break_on_error(bool b)
	{
		impl::BreakOnError::s_value = b;
	}
	bool is_break_on_error_enabled()
	{
		return impl::BreakOnError::s_value;
	}
}

#if _DEBUG

#include <iostream>	// std::cout

namespace my_gl_core
{
	namespace impl
	{
		bool CheckOpenGLError(GLenum error)
		{
			if (error == 0)
				return false;

			const char * error_name = "_unknown_name_";

			switch (error)
			{
			case gl::INVALID_ENUM: error_name = "gl::INVALID_ENUM"; break;
			case gl::INVALID_VALUE: error_name = "gl::INVALID_ENUM"; break;
			case gl::INVALID_OPERATION: error_name = "gl::INVALID_ENUM"; break;
			case gl::OUT_OF_MEMORY: error_name = "gl::OUT_OF_MEMORY"; break;
			case gl::INVALID_FRAMEBUFFER_OPERATION: error_name = "gl::INVALID_FRAMEBUFFER_OPERATION"; break;
			}

			std::cout << "OpenGL error occurred: " << error_name << " #" << error << std::endl;

			return BreakOnError::s_value;
		}
	}
}
#endif

// compile in here the OpenGL functions
// (this way we can keep it in the external folder, it would be better not to do this)
#include "gl_core\gl_core_4_2.cpp"