#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FANN/fann.h"

#include "math.h"
#include "Randomizer.h"

class ANNTrainer;
class ANNWrapper;
class PhysicsBody;
class PhysicsManager;
class GraphicsManager;

struct ContactInfo;
struct JumpData;

class GameScene
{
public:
	GameScene(GraphicsManager& graphicsMgr, const ANNTrainer& trainer);
	~GameScene();
	void Update(float dt);

private:
	void UpdateMainEntity(float dt);
	void UpdateBulletEntity(float dt);
	void ContactEnterCallback(const ContactInfo & contactInfo);
	void ContactExitCallback(const ContactInfo & contactInfo);

	void StartGame();
	void RestartGame();
	void EndGame();

	bool	m_isPlayerOnGround;
	bool	m_gameRestarting;

	GraphicsManager&				m_graphicsMgr;

	std::unique_ptr<ANNWrapper>		m_ann;
	std::unique_ptr<PhysicsManager> m_physicsMgr;

	std::shared_ptr<PhysicsBody>	m_mainEntity;
	std::shared_ptr<PhysicsBody>	m_bulletEntity;
};