/*!
\author Borja Portugal Martin
*/

#ifndef IMGUI_SYSTEM_H_
#define IMGUI_SYSTEM_H_

#include <memory>	// std::unique_ptr

namespace app
{
	class Window;

	/// \brief	Class that manages all the ImGui resources, updates it and renders it.
	class ImGuiSystem
	{
	public:
		ImGuiSystem();
		~ImGuiSystem();
		void Update(Window & window);
		void Render() const;

	private:
		class ImGuiSystem_impl;
		std::unique_ptr<ImGuiSystem_impl>	mpImpl;

	};

}

#endif	// IMGUI_SYSTEM_H_