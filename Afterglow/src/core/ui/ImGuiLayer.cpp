#include "ImGuiLayer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <array>

namespace Afterglow
{
	namespace Core
	{
		namespace UI
		{
			ImGuiLayer::ImGuiLayer(GLFWwindow* mainWindow, bool enableDarkMode) : m_ContextWindow(mainWindow), m_DarkMode(enableDarkMode)
			{
				// Setup Dear ImGui context
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;	

				io.IniFilename = "afterglow_ui_configuration.ini";

				io.Fonts->AddFontFromFileTTF("res/fonts/bahnschrift.ttf", 16.0f);

				if (m_DarkMode)
					ImGui::StyleColorsDark();
				else
					ImGui::StyleColorsLight();

				ImGui_ImplGlfw_InitForOpenGL(m_ContextWindow, true);
				ImGui_ImplOpenGL3_Init("#version 330 core");
			}

			ImGuiLayer::~ImGuiLayer() 
			{
				// Shutdown ImGui and release resources
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
			}

			ImGuiLayer& ImGuiLayer::GetInstance(GLFWwindow* mainWindow, bool enableDarkMode)
			{
				static ImGuiLayer imGuiLayerInstance(mainWindow, enableDarkMode);
				return imGuiLayerInstance;
			}

			void ImGuiLayer::Update(float deltaTime, Scene& currentScene)
			{
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				currentScene.SceneImGui();
				ImGui::ShowDemoWindow();

				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}
		}
	}
}