#include "GUIManager.h"

#include <GL/glew.h>

#include "AppWindow.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

GUIManager::GUIManager(GameScene & scene, AppWindow & appWin) :
	m_gameScene(scene),
	m_appWin(appWin)
{
	// imgui stuff here
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// dark color
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_appWin.GetNativeWindow(), true);

	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);
}

GUIManager::~GUIManager()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void GUIManager::Update(float dt)
{

}

void GUIManager::BegFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GUIManager::EndFrame()
{
	ImGui::Render();

	// rendering here
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// programming tip: always reset state - application will be easier to debug ...
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void GUIManager::Render()
{
	BegFrame();

	// rendering here
	ImGui::Begin("Hello world");
	ImGui::End();

	EndFrame();
}