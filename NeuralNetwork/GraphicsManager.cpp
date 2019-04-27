#include "GraphicsManager.h"

#include "DebugDrawer.h"
#include "Camera.h"

GraphicsManager::GraphicsManager() :
	m_mainCam(std::make_unique<Camera>()),
	m_debugDrawer(std::make_unique<DebugDrawer>(*this))
{

}

void GraphicsManager::Render() const
{
	m_debugDrawer->RenderDebugShapes();
}

void GraphicsManager::StartFrame() const
{
	glClearColor(0.15f, 0.15f, 0.15f, 1.f); // clear drawing surface with this color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear framebuffer (color and depth)
}

void GraphicsManager::EndFrame() const
{
	// clean up
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void GraphicsManager::EnableAlphaBlend() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

DebugDrawer & GraphicsManager::GetDebugDrawer()
{
	return *m_debugDrawer;
}

DebugDrawer const & GraphicsManager::GetDebugDrawer() const
{
	return *m_debugDrawer;
}
Camera & GraphicsManager::GetMainCamera()
{
	return *m_mainCam;
}

Camera const & GraphicsManager::GetMainCamera() const
{
	return *m_mainCam;
}

void GraphicsManager::SetViewport(int minX, int minY, int maxX, int maxY)
{
	glViewport(minX, minY, maxX, maxY);

	int width = maxX - minX;
	int height = maxY - minY;
	m_mainCam->SetOrtho(width * 0.5f, width * -0.5f, height * -0.5f, height * 0.5f, -0.1f, 100.f);
}

const float gQuadVertices[20] =
{
	// positions			// texture coords
	// positions			// texture Coords
	-0.5f,  -0.5f,	0.0f,	0.0f, 1.0f,	// 0
	-0.5f,	0.5f,	0.0f,	0.0f, 0.0f,	// 1
	0.5f,	-0.5f,	0.0f,	1.0f, 1.0f,	// 2
	0.5f,	0.5f,	0.0f,	1.0f, 0.0f	// 3
};

unsigned int gQuadIndices[6] = // note that we start from 0!
{
	1, 0, 3,  // first Triangle
	1, 2, 3   // second Triangle
};

float gLineVertices[20] =
{
	 -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	  0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
	  0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
	 -0.5f, 0.5f, 0.0f, 0.0f, 0.0f
};

unsigned int gLineIndices[2] =  // note that we start from 0!
{
	0, 1
};
