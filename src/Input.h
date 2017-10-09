
#pragma once

#include <functional>	// std::function
#include <memory>		// std::unique_ptr

namespace app
{
	class Input
	{
	private:
		friend class Window;
		Input();

	public:
		// Type of the callbacks that can be set in order to be notified when an input happens.
		using key_callback = std::function<void(unsigned char key)>;
		using mouse_callback = std::function<void(unsigned char button, int x, int y)>;

		// Handy values to get the 
		enum MouseButtons
		{
			MOUSE_L = 1,
			MOUSE_R = 3,
			MOUSE_WHEEL = 2
		};

	public:

		/// \return X coordinate of the mouse position in window coordinates.
		int getMouseX() const;
		/// \return Y coordinate of the mouse position in window coordinates.
		int getMouseY() const;
		/// \return The number of keyboard keys that the input handles.
		std::size_t getKeyNum() const;
		/// \return The number of mouse buttons that the input handles.
		std::size_t getMouseButtonNum() const;

		/// \return True the first frame that the input keyboard key started been pressed.
		bool KeyTriggered(unsigned k) const;
		/// \return True while the input keyboard key is pressed.
		bool KeyPressed(unsigned k) const;
		/// \return True the first frame that the input mouse button started been pressed.
		bool MouseTriggered(unsigned b) const;
		/// \return True while the input mouse button is pressed.
		bool MousePressed(unsigned b) const;

		/// \brief	Callback is called the frame that the user presses a keyboard key.
		void setKeyTriggeredCallBack(key_callback key_triggered_callback);
		/// \brief	Callback is called all the frames a keyboard key is pressed.
		void setKeyPressedCallBack(key_callback key_pressed_callback);
		/// \brief	Callback is called when a keyboard key was pressed the prvious frame but not this one.
		void setKeyReleasedCallBack(key_callback key_released_callback);
		/// \brief	Callback is called the frame that the user presses a mouse button.
		void setMouseTriggeredCallBack(mouse_callback mouse_triggered_callback);
		/// \brief	Callback is called all the frames a mouse button is pressed.
		void setMousePressedCallBack(mouse_callback mouse_pressed_callback);
		/// \brief	Callback is called when a mouse button was pressed the prvious frame but not this one.
		void setMouseReleasedCallBack(mouse_callback mouse_released_callback);

	private:
		// Use pimpl pattern in order to hide the underlying window API.
		class Input_impl;
		std::unique_ptr<Input_impl> mpInputImpl;
	};
}

