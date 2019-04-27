#pragma once

class GameScene;
class AppWindow;

class GUIManager
{
public:
	GUIManager(GameScene & scene, AppWindow & appWin);
	~GUIManager();
	void Update(float dt);
	void Render();

private:
	void BegFrame();
	void EndFrame();
	AppWindow & m_appWin;
	GameScene & m_gameScene;
};