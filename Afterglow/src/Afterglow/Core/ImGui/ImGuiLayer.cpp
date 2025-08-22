#include "agpch.h"
#include "ImGuiLayer.h"
#include "Afterglow/Core/Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

namespace Afterglow
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGui")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}
	
	void ImGuiLayer::OnAttach()
	{
		// Dear ImGui context setup.
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;         // Enable Keyboard Controls.
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;             // Enable Docking.
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;           // Enable Multi-Viewport/Platform Windows.

		// Dear ImGui style.
		ImGui::StyleColorsDark();

		Application& app = Application::Get();
		auto window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Platform/Renderer bindings.
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		event.b_Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
		event.b_Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}