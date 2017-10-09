/*!
\author Borja Portugal Martin
\brief	We are implementing here both classes, Window and Input in order for Window_impl to store an Input object.
Headers are separated though so that the user can include what he uses.
*/

#include "Window.h"		// Window
#include "Input.h"		// Input

#include "SDL\SDL.h"	// SDL functions
#include "my_gl_core.h"	// namespace gl

#include <stdexcept>	// std::runtime_error
#include <memory>		// std::uniuqe_ptr, std::make_unique
#include <array>		// std::array

#include <iostream>		// std::cout
#include <cctype>		// std::tolower

namespace app
{
#pragma region // Input_impl
	class Input::Input_impl
	{
	public:
		void Update();
		bool ProcessEvent(const SDL_Event & event);

		bool KeyTriggered(const unsigned k) const;
		bool KeyPressed(const unsigned k) const;
		bool MouseTriggered(const unsigned b) const;
		bool MousePressed(const unsigned b) const;

		std::size_t getKeyNum() const { return mvKeyboardKeys.size(); }
		std::size_t getMouseButtonNum() const { return mvMouseButtons.size(); }

		int getMouseX() const { return mMouse_x; }
		int getMouseY() const { return mMouse_y; }

		void setKeyTriggeredCallBack(key_callback key_triggered_callback);
		void setKeyPressedCallBack(key_callback key_pressed_callback);
		void setKeyReleasedCallBack(key_callback key_released_callback);
		void setMouseTriggeredCallBack(mouse_callback mouse_triggered_callback);
		void setMousePressedCallBack(mouse_callback mouse_pressed_callback);
		void setMouseReleasedCallBack(mouse_callback mouse_released_callback);

	private:
		int getKeyState(unsigned k) const;
		int getButtonState(unsigned b) const;

		static unsigned UpdateKeyState(unsigned char & prev, unsigned char & curr);
		static void DummyKeyCallBack(unsigned char) {}
		static void DummyMouseCallBack(unsigned char, int, int) {}

		static const unsigned char PRESSED_FLAG = 1 << 0;
		static const unsigned char TRIGGERED_FLAG = 1 << 1;
		static const unsigned char RELEASED_FLAG = 1 << 2;

		/// \brief	Stores the previous and current state of a key to determine
		/// the state (i.e. Trigger, Pressed or Released)
		struct CurrPrev
		{
			CurrPrev() : mCurr(0), mPrev(0) {}
			unsigned char mCurr;
			unsigned char mPrev;
		};

		std::array<CurrPrev, 256u> mvKeyboardKeys;
		std::array<CurrPrev, 4u> mvMouseButtons;

		// mouse position
		int mMouse_x{ 0 }, mMouse_y{ 0 };

		key_callback mKeyTriggeredCallBack{ DummyKeyCallBack };
		key_callback mKeyPressededCallBack{ DummyKeyCallBack };
		key_callback mKeyReleasedCallBack{ DummyKeyCallBack };

		mouse_callback mMouseTriggeredCallBack{ DummyMouseCallBack };
		mouse_callback mMousePressededCallBack{ DummyMouseCallBack };
		mouse_callback mMouseReleasedCallBack{ DummyMouseCallBack };
	};

	bool Input::Input_impl::ProcessEvent(const SDL_Event & event)
	{
		const unsigned k = std::tolower(SDL_GetKeyFromScancode(event.key.keysym.scancode));
		switch (event.type)
		{
			// keyboard
		case SDL_KEYDOWN:
		{
			if (k < mvKeyboardKeys.size())
				mvKeyboardKeys[k].mCurr = 1;
		} break;
		case SDL_KEYUP:
		{
			if (k < mvKeyboardKeys.size())
				mvKeyboardKeys[k].mCurr = 0;
		} break;

		// mouse
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event.button.button < mvMouseButtons.size())
				mvMouseButtons[event.button.button].mCurr = 1;
		} break;
		case SDL_MOUSEBUTTONUP:
		{
			if (event.button.button < mvMouseButtons.size())
				mvMouseButtons[event.button.button].mCurr = 0;
		} break;
		case SDL_MOUSEMOTION:
		{
			mMouse_x = event.motion.x;
			mMouse_y = event.motion.y;
		} break;
		default:
		{
			return false;
		} break;
		}

		return true;
	}
	unsigned Input::Input_impl::UpdateKeyState(unsigned char & prev, unsigned char & curr)
	{
		unsigned flags = 0;

		// pressed  this frame ??
		if (curr == 1)
		{
			// not pressed previous frame ??
			if (prev == 0)
			{
				curr = 2;	// trigger

				flags |= TRIGGERED_FLAG;
			}
		}
		else if (curr == 2)	// previous frame was trigger
			curr = 1; // now is pressed
		else if (curr == 0 && prev > 0)	// key was released
			flags |= RELEASED_FLAG;

		prev = curr;

		if (curr > 0)
			flags |= PRESSED_FLAG;

		return flags;
	}
	void Input::Input_impl::Update()
	{
		for (unsigned i = 0; i < mvKeyboardKeys.size(); ++i)
		{
			const unsigned flags = UpdateKeyState(mvKeyboardKeys[i].mPrev, mvKeyboardKeys[i].mCurr);

			if (flags & TRIGGERED_FLAG) mKeyTriggeredCallBack(static_cast<unsigned char>(i));
			if (flags & PRESSED_FLAG)	mKeyPressededCallBack(static_cast<unsigned char>(i));
			if (flags & RELEASED_FLAG)	mKeyReleasedCallBack(static_cast<unsigned char>(i));
		}

		for (unsigned i = 0; i < mvMouseButtons.size(); ++i)
		{
			const unsigned flags = UpdateKeyState(mvMouseButtons[i].mPrev, mvMouseButtons[i].mCurr);

			if (flags & TRIGGERED_FLAG) mMouseTriggeredCallBack(static_cast<unsigned char>(i), mMouse_x, mMouse_y);
			if (flags & PRESSED_FLAG)	mMousePressededCallBack(static_cast<unsigned char>(i), mMouse_x, mMouse_y);
			if (flags & RELEASED_FLAG)	mMouseReleasedCallBack(static_cast<unsigned char>(i), mMouse_x, mMouse_y);
		}
	}

	bool Input::Input_impl::KeyTriggered(const unsigned k) const
	{
		return getKeyState(k) == 2;
	}
	bool Input::Input_impl::KeyPressed(const unsigned k) const
	{
		return getKeyState(k) > 0;
	}
	bool Input::Input_impl::MouseTriggered(const unsigned b) const
	{
		return getButtonState(b) == 2;
	}
	bool Input::Input_impl::MousePressed(const unsigned b) const
	{
		return getButtonState(b) > 0;
	}

	int Input::Input_impl::getKeyState(unsigned k) const
	{
		k = std::tolower(k);
		if (k < mvKeyboardKeys.size())
			return mvKeyboardKeys[k].mCurr;
		return 0;
	}
	int Input::Input_impl::getButtonState(unsigned b) const
	{
		if (b < mvMouseButtons.size())
			return mvMouseButtons[b].mCurr;
		return 0;
	}

	void Input::Input_impl::setKeyTriggeredCallBack(key_callback key_triggered_callback)
	{
		mKeyTriggeredCallBack = key_triggered_callback ? key_triggered_callback : DummyKeyCallBack;
	}
	void Input::Input_impl::setKeyPressedCallBack(key_callback key_pressed_callback)
	{
		mKeyPressededCallBack = key_pressed_callback ? key_pressed_callback : DummyKeyCallBack;
	}
	void Input::Input_impl::setKeyReleasedCallBack(key_callback key_released_callback)
	{
		mKeyReleasedCallBack = key_released_callback ? key_released_callback : DummyKeyCallBack;
	}
	void Input::Input_impl::setMouseTriggeredCallBack(mouse_callback mouse_triggered_callback)
	{
		mMouseTriggeredCallBack = mouse_triggered_callback ? mouse_triggered_callback : DummyMouseCallBack;
	}
	void Input::Input_impl::setMousePressedCallBack(mouse_callback mouse_pressed_callback)
	{
		mMousePressededCallBack = mouse_pressed_callback ? mouse_pressed_callback : DummyMouseCallBack;
	}
	void Input::Input_impl::setMouseReleasedCallBack(mouse_callback mouse_released_callback)
	{
		mMouseReleasedCallBack = mouse_released_callback ? mouse_released_callback : DummyMouseCallBack;
	}
#pragma endregion

#pragma region // Input
	
	Input::Input()
		: mpInputImpl(std::make_unique<Input_impl>())
	{}

	std::size_t Input::getKeyNum() const
	{
		return mpInputImpl->getKeyNum();
	}
	std::size_t Input::getMouseButtonNum() const
	{
		return mpInputImpl->getMouseButtonNum();
	}
	int Input::getMouseX() const
	{
		return mpInputImpl->getMouseX();
	}
	int Input::getMouseY() const
	{
		return mpInputImpl->getMouseY();
	}

	bool Input::KeyTriggered(unsigned k) const
	{
		return mpInputImpl->KeyTriggered(k);
	}
	bool Input::KeyPressed(unsigned k) const
	{
		return mpInputImpl->KeyPressed(k);
	}
	bool Input::MouseTriggered(unsigned b) const
	{
		return mpInputImpl->MouseTriggered(b);
	}
	bool Input::MousePressed(unsigned b) const
	{
		return mpInputImpl->MousePressed(b);
	}

	void Input::setKeyTriggeredCallBack(key_callback key_triggered_callback)
	{
		mpInputImpl->setKeyTriggeredCallBack(key_triggered_callback);
	}
	void Input::setKeyPressedCallBack(key_callback key_pressed_callback)
	{
		mpInputImpl->setKeyPressedCallBack(key_pressed_callback);
	}
	void Input::setKeyReleasedCallBack(key_callback key_released_callback)
	{
		mpInputImpl->setKeyReleasedCallBack(key_released_callback);
	}
	void Input::setMouseTriggeredCallBack(mouse_callback mouse_triggered_callback)
	{
		mpInputImpl->setMouseTriggeredCallBack(mouse_triggered_callback);
	}
	void Input::setMousePressedCallBack(mouse_callback mouse_pressed_callback)
	{
		mpInputImpl->setMousePressedCallBack(mouse_pressed_callback);
	}
	void Input::setMouseReleasedCallBack(mouse_callback mouse_released_callback)
	{
		mpInputImpl->setMouseReleasedCallBack(mouse_released_callback);
	}
#pragma endregion

#pragma region // Window_impl
	class Window::Window_impl
	{
	public:
		Window_impl(const char * name, int w, int h);
		~Window_impl();

		bool Update();
		void SwapBuffers();
		void Close();

		float getDt() const { return mDt; }
		int getWindowWidth() const { return mWidth; }
		int getWindowHeight() const { return mHeight; }
		Input & getInput() { return mInput; }
		bool isOpened() const { return mbOpened; }

	private:
		void ProcessEvent(const SDL_WindowEvent & window_event);

		int mWidth{ 0 };
		int mHeight{ 0 };
		bool mbOpened{ true };

		SDL_Window * mpSDL_Window{ nullptr };
		SDL_GLContext mpGLContext{ nullptr };

		Input mInput;
		float mDt{ 0.f };
		unsigned mLastTicks{ 0u };
	};

	Window::Window_impl::Window_impl(const char * name, int w, int h)
		: mWidth(w)
		, mHeight(h)
	{
		mpSDL_Window = SDL_CreateWindow(name,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			w,
			h,
			SDL_WINDOW_OPENGL);

		if (!mpSDL_Window)
			throw std::runtime_error{ "SDL couldn't be initialize SDL!" };

		// create open gl context and load functions
		mpGLContext = SDL_GL_CreateContext(mpSDL_Window);

		if (!mpGLContext)
		{
			SDL_DestroyWindow(mpSDL_Window);
			throw std::runtime_error{ "OpenGL context couldn't be created!" };
		}

		const auto gl_sys_loaded = gl::sys::LoadFunctions();
		if (!gl_sys_loaded)
		{
			SDL_GL_DeleteContext(mpGLContext);
			SDL_DestroyWindow(mpSDL_Window);
			throw std::runtime_error{ "OpenGL functions couldn't be loaded." };
		}

		std::cout << std::endl
			<< "------------------- OpenGL -------------------" << std::endl
			<< "Number of gl functions that failed to load: " << gl_sys_loaded.GetNumMissing() << std::endl
			<< "GL Vendor: " << gl::GetString(gl::VENDOR) << std::endl
			<< "GL Renderer: " << gl::GetString(gl::RENDERER) << std::endl
			<< "GL Version: " << gl::GetString(gl::VERSION) << std::endl
			<< "GLSL Version: " << gl::GetString(gl::SHADING_LANGUAGE_VERSION) << std::endl
			<< "----------------------------------------------" << std::endl
			<< std::endl;

	}
	Window::Window_impl::~Window_impl()
	{
		if (mpSDL_Window)
		{
			SDL_GL_DeleteContext(mpGLContext);
			mpGLContext = nullptr;

			SDL_DestroyWindow(mpSDL_Window);
			mpSDL_Window = nullptr;
		}
	}

	bool Window::Window_impl::Update()
	{
		if (!isOpened())	return false;

		// update dt
		const unsigned curr_ticks = SDL_GetTicks();
		mDt = (curr_ticks - mLastTicks) / 1000.f;
		mLastTicks = curr_ticks;

		// pool all the events
		SDL_Event sdl_event;
		while (SDL_PollEvent(&sdl_event))
		{
			if (mInput.mpInputImpl->ProcessEvent(sdl_event))
				continue;

			switch (sdl_event.type)
			{
			case SDL_WINDOWEVENT:
			{
				ProcessEvent(sdl_event.window);
			} break;
			}
		}

		mInput.mpInputImpl->Update();
		return true;
	}
	void Window::Window_impl::ProcessEvent(const SDL_WindowEvent & window_event)
	{
		switch (window_event.event)
		{
		case SDL_WINDOWEVENT_CLOSE:
		{
			Close();
		} break;
		}
	}
	void Window::Window_impl::SwapBuffers()
	{
		SDL_GL_SwapWindow(mpSDL_Window);
	}
	void Window::Window_impl::Close()
	{
		mbOpened = false;
	}
#pragma endregion

#pragma region // Window
	Window::Window(const char * name, int w, int h)
		: mpWindowImpl(std::make_unique<Window_impl>(name, w, h))
	{}
	Window::~Window() {}

	bool Window::Update()
	{
		return mpWindowImpl->Update();
	}
	void Window::SwapBuffers()
	{
		mpWindowImpl->SwapBuffers();
	}
	void Window::Close()
	{
		mpWindowImpl->Close();
	}

	float Window::getDt() const
	{
		return mpWindowImpl->getDt();
	}
	Input & Window::getInput() const
	{
		return mpWindowImpl->getInput();
	}
	int Window::getWindowWidth() const
	{
		return mpWindowImpl->getWindowWidth();
	}
	int Window::getWindowHeight() const
	{
		return mpWindowImpl->getWindowHeight();
	}
	bool Window::isOpened() const
	{
		return mpWindowImpl->isOpened();
	}
#pragma endregion

	// TODO(Borja): Be able to pass parametters
	void Initialize(int opengl_mayor, int opengl_minor,
					int depth_size, int stencil_size)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opengl_mayor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opengl_minor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth_size);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencil_size);

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			throw std::runtime_error{ "SDL could not initialize SDL!" };
	}
	void Shutdown()
	{
		SDL_Quit();
	}
}
