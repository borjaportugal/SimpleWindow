/*!
\author Borja Portugal Martin
*/

#include "ImGuiSystem.h"

#include "Window.h"
#include "Input.h"

#include "GUI.h"		// namespace ImGui, GUI_ENABLED

// compile imgui here so that we don't need to include it in the project 
// (this way we can keep it in the external folder, it would be better not to do this)
#include "imgui\imgui.cpp"
#include "imgui\imgui_draw.cpp"
#include "imgui\imgui_demo.cpp"

#include "my_gl_core.h"

namespace app
{
	class ImGuiSystem::ImGuiSystem_impl
	{
		/// \brief	Initialices ImGui and allocates all the needed resources.
		void Init();
		void Shutdown();

		/// \brief	Renders the ImGui windows.
		void RenderDrawLists(ImDrawData* draw_data);
		/// \brief	Allocates the ImGui fonts.
		void CreateFontsTexture();
		/// \brief	Creates the shaders that Imgui is going to be using.
		void CreateDeviceObjects();
	public:
		ImGuiSystem_impl();
		~ImGuiSystem_impl();
		void Render();

		/// \brief	Begins a new ImGui frame.
		void NewFrame(Window & window);

	private:
		// variables needed by ImGui
		double       g_Time = 0.0f;
		float        g_MouseWheel = 0.0f;
		GLuint       g_FontTexture = 0;
		int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
		int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
		int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;

		unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

		bool mbVisible{ true };
	};

	ImGuiSystem::ImGuiSystem_impl::ImGuiSystem_impl()
	{
		Init();
		CreateDeviceObjects();
	}
	ImGuiSystem::ImGuiSystem_impl::~ImGuiSystem_impl()
	{
		Shutdown();
	}

	void ImGuiSystem::ImGuiSystem_impl::Init()
	{
		// get window handle from SDL
		{
			//SDL_SysWMinfo systemInfo;
			//SDL_VERSION(&systemInfo.version);
			//SDL_GetWindowWMInfo(Window::getInstance().getWindow(), &systemInfo);
			//g_Window = systemInfo.info.win.window;
		}

		ImGuiIO& io = ImGui::GetIO();
		// the OP magic numbers, better if we serialize this in the InputManager xD
		io.KeyMap[ImGuiKey_Tab] = 9;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = 38;
		io.KeyMap[ImGuiKey_DownArrow] = 40;
		io.KeyMap[ImGuiKey_PageUp] = 33;
		io.KeyMap[ImGuiKey_PageDown] = 34;
		io.KeyMap[ImGuiKey_Home] = 36;
		io.KeyMap[ImGuiKey_End] = 35;
		io.KeyMap[ImGuiKey_Delete] = 127;
		io.KeyMap[ImGuiKey_Backspace] = 8;
		io.KeyMap[ImGuiKey_Enter] = 13;
		io.KeyMap[ImGuiKey_Escape] = 27;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';

		io.RenderDrawListsFn = nullptr;
#ifdef _WIN32
		//io.ImeWindowHandle = g_Window;
#endif
	}

	void ImGuiSystem::ImGuiSystem_impl::CreateDeviceObjects()
	{
		// Backup GL state
		GLint last_texture, last_array_buffer, last_vertex_array;
		gl::GetIntegerv(gl::TEXTURE_BINDING_2D, &last_texture);
		gl::GetIntegerv(gl::ARRAY_BUFFER_BINDING, &last_array_buffer);
		gl::GetIntegerv(gl::VERTEX_ARRAY_BINDING, &last_vertex_array);

		const GLchar *vertex_shader =
			"#version 330\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"#version 330\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		g_ShaderHandle = gl::CreateProgram();
		g_VertHandle = gl::CreateShader(gl::VERTEX_SHADER);
		g_FragHandle = gl::CreateShader(gl::FRAGMENT_SHADER);
		gl::ShaderSource(g_VertHandle, 1, &vertex_shader, 0);
		gl::ShaderSource(g_FragHandle, 1, &fragment_shader, 0);
		gl::CompileShader(g_VertHandle);
		gl::CompileShader(g_FragHandle);
		gl::AttachShader(g_ShaderHandle, g_VertHandle);
		gl::AttachShader(g_ShaderHandle, g_FragHandle);
		gl::LinkProgram(g_ShaderHandle);

		g_AttribLocationTex = gl::GetUniformLocation(g_ShaderHandle, "Texture");
		g_AttribLocationProjMtx = gl::GetUniformLocation(g_ShaderHandle, "ProjMtx");
		g_AttribLocationPosition = gl::GetAttribLocation(g_ShaderHandle, "Position");
		g_AttribLocationUV = gl::GetAttribLocation(g_ShaderHandle, "UV");
		g_AttribLocationColor = gl::GetAttribLocation(g_ShaderHandle, "Color");

		gl::GenBuffers(1, &g_VboHandle);
		gl::GenBuffers(1, &g_ElementsHandle);

		gl::GenVertexArrays(1, &g_VaoHandle);
		gl::BindVertexArray(g_VaoHandle);
		gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
		gl::EnableVertexAttribArray(g_AttribLocationPosition);
		gl::EnableVertexAttribArray(g_AttribLocationUV);
		gl::EnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		gl::VertexAttribPointer(g_AttribLocationPosition, 2, gl::FLOAT, gl::FALSE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
		gl::VertexAttribPointer(g_AttribLocationUV, 2, gl::FLOAT, gl::FALSE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
		gl::VertexAttribPointer(g_AttribLocationColor, 4, gl::UNSIGNED_BYTE, gl::TRUE_, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

		CreateFontsTexture();

		// Restore modified GL state
		gl::BindTexture(gl::TEXTURE_2D, last_texture);
		gl::BindBuffer(gl::ARRAY_BUFFER, last_array_buffer);
		gl::BindVertexArray(last_vertex_array);
	}
	void ImGuiSystem::ImGuiSystem_impl::CreateFontsTexture()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Build texture atlas
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

		// Create OpenGL texture
		gl::GenTextures(1, &g_FontTexture);
		gl::BindTexture(gl::TEXTURE_2D, g_FontTexture);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, width, height, 0, gl::RGBA, gl::UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

		// Cleanup (don't clear the input data if you want to append new fonts later)
		io.Fonts->ClearInputData();
		io.Fonts->ClearTexData();
	}
	void ImGuiSystem::ImGuiSystem_impl::Shutdown()
	{
		if (g_VaoHandle)		gl::DeleteVertexArrays(1, &g_VaoHandle);
		if (g_VboHandle)		gl::DeleteBuffers(1, &g_VboHandle);
		if (g_ElementsHandle)	gl::DeleteBuffers(1, &g_ElementsHandle);
		g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

		gl::DetachShader(g_ShaderHandle, g_VertHandle);
		gl::DeleteShader(g_VertHandle);
		g_VertHandle = 0;

		gl::DetachShader(g_ShaderHandle, g_FragHandle);
		gl::DeleteShader(g_FragHandle);
		g_FragHandle = 0;

		gl::DeleteProgram(g_ShaderHandle);
		g_ShaderHandle = 0;

		if (g_FontTexture)
		{
			gl::DeleteTextures(1, &g_FontTexture);
			ImGui::GetIO().Fonts->TexID = 0;
			g_FontTexture = 0;
		}
		ImGui::Shutdown();
	}

	void ImGuiSystem::ImGuiSystem_impl::NewFrame(Window & window)
	{
		const float dt = window.getDt();
		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)

		const int w = window.getWindowWidth();
		const int h = window.getWindowHeight();
		const int display_w = w;
		const int display_h = h;

		//io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

		// Setup time step
		double current_time = dt;
		io.DeltaTime = g_Time > 0.0 ? (float)(current_time) : (float)(1.0f / 60.0f);
		g_Time = current_time;

		const Input & input = window.getInput();

		// TODO(Borja): handle capital letters
		// TODO(Borja): handle symbol input
		// keyboard data
		for (unsigned short i = 1; i < input.getKeyNum(); i++)
		{
			io.KeysDown[i] = input.KeyTriggered(i);
			if (io.KeysDown[i])
				io.AddInputCharacter(i);
		}

		// mouse
	{
		// TODO(Borja): check if mouse is inside the window
		if ([]{ return true; }())
		{
			const float x = static_cast<float>(input.getMouseX());
			const float y = static_cast<float>(input.getMouseY());
			io.MousePos = ImVec2(x, y);   // Mouse position in screen coordinates (set to -1,-1 //if /no	mouse / on another screen, etc.)
		}
		else
		{
			io.MousePos = ImVec2(-1, -1);
		}

		// TODO(Borja): Mouse wheel
		//io.MouseWheel = static_cast<float>(input.getMouseWheel());
		for (unsigned short i = 1; i < input.getMouseButtonNum(); i++)
		{
			io.MouseDown[i - 1] = input.MousePressed(i);
		}

		//// if the mouse is on top a window, set our inputs to nothing
		//input.setInputActive(!ImGui::IsAnyItemActive());
		//input.setMouseInputActive(!ImGui::IsMouseHoveringAnyWindow());
	}

		// Start the frame
		ImGui::NewFrame();
	}
	void ImGuiSystem::ImGuiSystem_impl::Render()
	{
		// render the GUI
		ImGui::Render();
		if (ImDrawData * pImDrawData = ImGui::GetDrawData())
			RenderDrawLists(pImDrawData);
	}

	void ImGuiSystem::ImGuiSystem_impl::RenderDrawLists(ImDrawData* draw_data)
	{
		if (draw_data == nullptr)
			return;

		// Backup GL state
		GLint last_program, last_texture, last_array_buffer, last_element_array_buffer, last_vertex_array;
		gl::GetIntegerv(gl::CURRENT_PROGRAM, &last_program);
		gl::GetIntegerv(gl::TEXTURE_BINDING_2D, &last_texture);
		gl::GetIntegerv(gl::ARRAY_BUFFER_BINDING, &last_array_buffer);
		gl::GetIntegerv(gl::ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
		gl::GetIntegerv(gl::VERTEX_ARRAY_BINDING, &last_vertex_array);

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
		gl::Enable(gl::BLEND);

		gl::BlendEquationi(0, gl::FUNC_ADD);
		gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

		/*GLboolean cull_face_enabled, depth_test_enabled, scissor_test_enabled;
		gl::GetBooleani_v(gl::CULL_FACE, 0, &cull_face_enabled);
		gl::GetBooleani_v(gl::DEPTH_TEST, 0, &depth_test_enabled);
		gl::GetBooleani_v(gl::SCISSOR_TEST, 0, &scissor_test_enabled);
		GLint last_active_texture;
		gl::GetIntegeri_v(gl::ACTIVE_TEXTURE, 0, &last_active_texture);*/

		gl::Disable(gl::CULL_FACE);
		gl::Disable(gl::DEPTH_TEST);
		gl::Enable(gl::SCISSOR_TEST);
		gl::ActiveTexture(gl::TEXTURE0);

		// Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
		ImGuiIO& io = ImGui::GetIO();
		float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		// Setup orthographic projection matrix
		const float ortho_projection[4][4] =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f, 0.0f, -1.0f, 0.0f },
			{ -1.0f, 1.0f, 0.0f, 1.0f },
		};
		gl::UseProgram(g_ShaderHandle);
		gl::Uniform1i(g_AttribLocationTex, 0);
		gl::UniformMatrix4fv(g_AttribLocationProjMtx, 1, gl::FALSE_, &ortho_projection[0][0]);
		gl::BindVertexArray(g_VaoHandle);

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			gl::BindBuffer(gl::ARRAY_BUFFER, g_VboHandle);
			gl::BufferData(gl::ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), gl::STREAM_DRAW);

			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
			gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), gl::STREAM_DRAW);

			for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
			{
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					GLuint id = static_cast<GLuint>(reinterpret_cast<std::uintptr_t>(pcmd->TextureId));
					gl::BindTexture(gl::TEXTURE_2D, id);
					gl::Scissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
					gl::DrawElements(gl::TRIANGLES, (GLsizei)pcmd->ElemCount, gl::UNSIGNED_SHORT, idx_buffer_offset);
				}
				idx_buffer_offset += pcmd->ElemCount;
			}
		}

		// Restore modified GL state
		gl::UseProgram(last_program);
		gl::ActiveTexture(gl::TEXTURE0);
		gl::BindTexture(gl::TEXTURE_2D, last_texture);
		//gl::BindTexture(gl::TEXTURE_2D, last_texture_id);
		gl::BindBuffer(gl::ARRAY_BUFFER, last_array_buffer);
		gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
		gl::BindVertexArray(last_vertex_array);
		gl::Disable(gl::SCISSOR_TEST);
		gl::Enable(gl::DEPTH_TEST);

		gl::GetError();
	}

	ImGuiSystem::ImGuiSystem()
		: mpImpl(std::make_unique<ImGuiSystem_impl>())
	{}
	ImGuiSystem::~ImGuiSystem() {}

	void ImGuiSystem::Update(Window & window)
	{
		mpImpl->NewFrame(window);
	}
	void ImGuiSystem::Render() const
	{
		mpImpl->Render();
	}

}
