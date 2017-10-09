/*!
\author Borja Portugal Martin
*/

#pragma once

#include <memory>		// std::unique_ptr

// TODO(Borja): Window resize
// TODO(Borja): Fullscreen

namespace app
{
	// Needed by Window::getInput
	class Input;
	
	/// \brief	Wraps a window api and all the functionality of a window.
	/// IMPORTANT(Borja): Only supports the creation of one window.
	class Window
	{
	public:
		Window(const char * name, int w, int h);
		/// \brief Need a destructor because if the compiler generates it the Window_impl 
		/// destructor won't be accessible.
		~Window();
		Window(Window &&) = default;
		Window& operator=(Window &&) = default;
		Window(const Window &) = delete;
		Window& operator=(const Window &) = delete;
		
		/// \brief	Handle new inputs, returns if the window is still opened
		bool Update();
		void SwapBuffers();
		/// \brief After calling it Window::isOpened and Window::Update will return false, 
		/// the window won't be closed until the dtor of this class is called.
		void Close();

		/// \brief	Returns the delta time that the last Update recorded.
		float getDt() const;
		/// \brief	Returns the object that handles the input for this window.
		Input & getInput() const;

		int getWindowWidth() const;
		int getWindowHeight() const;

		/// \return False if Window::Close has been called.
		bool isOpened() const;

	private:
		// Use pimpl pattern in order to hide the underlying window API.
		class Window_impl;
		std::unique_ptr<Window_impl> mpWindowImpl;
	};
	
	/// \brief	This function needs to be called before creating a window object.
	void Initialize(int opengl_mayor, int opengl_minor,
					int depth_size = 24, int stencil_size = 1);
	/// \brief	This function needs to be called after destroying the window.
	void Shutdown();
}
