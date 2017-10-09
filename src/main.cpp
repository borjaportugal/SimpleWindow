
#include "Window.h"
#include "Input.h"

#include "my_gl_core.h"
#include "IMGUISystem.h"
#include "GUI.h"

#include <iostream>	// std::cout

void update(app::Window & window)
{
	const app::Input & input = window.getInput();

	if (input.KeyPressed('A'))		gl::ClearColor(1.f, 0.2f, 1.f, 1.f);
	if (input.KeyTriggered('S'))	gl::ClearColor(1.f, 1.2f, 0.f, 1.f);
	if (input.KeyTriggered('D'))	gl::ClearColor(0.f, 1.2f, 1.f, 1.f);

	if (input.MouseTriggered(app::Input::MOUSE_L))
		std::cout << ',' << '\n';
	if (input.MousePressed(app::Input::MOUSE_R))
		std::cout << 'e' << '\n';
	if (input.MouseTriggered(app::Input::MOUSE_WHEEL))
		std::cout << 'w' << '\n';
}

void render()
{
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);	CheckOGLError();
}

void key_triggered(unsigned char k)
{
	std::cout << k << '\n';
}

void run(const char * name, int w, int h, const unsigned char close_key)
{
	app::Window window{ name, w, h };
	app::ImGuiSystem imgui_sys;

	window.getInput().setKeyTriggeredCallBack(key_triggered);

	while (window.isOpened())
	{
		window.Update();
		imgui_sys.Update(window);

		ImGui::ShowTestWindow();

		update(window);
		render();
		imgui_sys.Render();

		window.SwapBuffers();

		if (window.getInput().KeyTriggered(close_key))
			window.Close();
	}
}

int main()
{
	try
	{
		app::Initialize(my_gl_core::get_opengl_mayor_v(), 
						my_gl_core::get_opengl_minor_v());

		const unsigned char scape = 27;
		run("Test Window", 1280, 720, scape);

		app::Shutdown();
	}
	catch (const std::exception & ex)
	{
		std::cout << "Exception caught on main: " << ex.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Something very bad happened!!" << std::endl;
	}
}
