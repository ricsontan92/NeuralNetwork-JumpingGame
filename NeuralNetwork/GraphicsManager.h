#pragma once
#include <memory>

extern const float	gQuadVertices[20];
extern unsigned int gQuadIndices[6];
extern float		gLineVertices[20];
extern unsigned int gLineIndices[2];

class DebugDrawer;
class Camera;

class GraphicsManager
{
public:
	GraphicsManager();
	void Render() const;
	void StartFrame() const;
	void EndFrame() const;

	// graphics utils
	void EnableAlphaBlend() const;

	// gettors
	DebugDrawer & GetDebugDrawer();
	DebugDrawer const & GetDebugDrawer() const;
	Camera & GetMainCamera();
	Camera const & GetMainCamera() const;

	// settors
	void SetViewport(int minX, int minY, int maxX, int maxY);
private:
	std::unique_ptr< DebugDrawer>	m_debugDrawer;
	std::unique_ptr< Camera>		m_mainCam;
};