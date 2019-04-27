#include "GameScene.h"
#include "JumpData.h"
#include "ANNTrainer.h"
#include "ANNWrapper.h"
#include "PhysicsBody.h"
#include "PhysicsManager.h"
#include "GraphicsManager.h"
#include "PhysicsContactListener.h"
#include <iostream>

GameScene::GameScene(GraphicsManager& graphicsMgr, const ANNTrainer& trainer) :
	m_physicsMgr(std::make_unique<PhysicsManager>(math::vec2(0.f, -9.8f), graphicsMgr.GetDebugDrawer())),
	m_graphicsMgr(graphicsMgr),
	m_gameRestarting(false),
	m_isPlayerOnGround(false)
{
	m_physicsMgr->GetContactListener().SetBeginContactCallbackFunction(&GameScene::ContactEnterCallback, this);
	m_physicsMgr->GetContactListener().SetEndContactCallbackFunction(&GameScene::ContactExitCallback, this);

	ANNWrapper::ANNConfig annConfig;
	annConfig.m_numInputs			= 2;
	annConfig.m_numOutputs			= 1;
	annConfig.m_numLayers			= 4;
	annConfig.m_numNeuronsInHidden	= 3;
	annConfig.m_maxErr				= 0.01f;
	annConfig.m_maxEpochs			= 500000;
	annConfig.m_epochsBtwnReports	= 50000;

	m_ann = std::make_unique<ANNWrapper>(annConfig);
	m_ann->SetTrainer(std::make_unique<ANNTrainer>(trainer));
	m_ann->Train();

	StartGame();
}

GameScene::~GameScene()
{
	m_physicsMgr->GetContactListener().ClearListenerFunctions();
	EndGame();
}

void GameScene::Update(float dt)
{
	if (m_gameRestarting)
	{
		EndGame();
		StartGame();
	}
	else
	{
		UpdateMainEntity(dt);
		UpdateBulletEntity(dt);
		m_physicsMgr->Update(dt);
		m_physicsMgr->RenderDebugShapes();
	}
}

void GameScene::UpdateMainEntity(float dt)
{
	if (!m_isPlayerOnGround)
	{
		return;
	}

	if (m_bulletEntity)
	{
		float inputs[(unsigned)JumpData::INPUT_TYPE::COUNT];
		inputs[(unsigned)JumpData::INPUT_TYPE::X_DIST] = m_mainEntity->GetPosition().x - m_bulletEntity->GetPosition().x;
		inputs[(unsigned)JumpData::INPUT_TYPE::X_SPEED] = m_bulletEntity->GetVelocity().x;
		std::vector<fann_type> outputs = m_ann->Run(inputs);

		if (outputs[(unsigned)JumpData::OUTPUT_TYPE::JUMP] >= 0.f)
		{
			m_mainEntity->AddForceToCenter(math::vec2(0.f, 3000.f));
			m_isPlayerOnGround = false;
		}
	}
}

void GameScene::UpdateBulletEntity(float dt)
{
	if (m_isPlayerOnGround && m_bulletEntity == nullptr)
	{
		static 	Randomizer bullerRandomizer(2000.f, 5000.f);
		m_bulletEntity = m_physicsMgr->AddBox(math::vec2(-500.f, -50.f), math::vec2(50.f, 90.f), 0.f, PhysicsManager::BodyType::DYNAMIC);
		m_bulletEntity->SetGravityScale(0.f);
		m_bulletEntity->AddForceToCenter(math::vec2(bullerRandomizer.GetRandomFloat(), 0.f));
		m_bulletEntity->SetName("Bullet");
	}
}

void GameScene::ContactEnterCallback(const ContactInfo & contactInfo)
{
	bool AisBullet			= "Bullet" == contactInfo.m_bodyA->GetName();
	bool BisBullet			= "Bullet" == contactInfo.m_bodyB->GetName();
	bool AisBulletDestroy	= "BulletDestroy" == contactInfo.m_bodyA->GetName();
	bool BisBulletDestroy	= "BulletDestroy" == contactInfo.m_bodyB->GetName();
	bool AisMainPlayer		= "MainPlayer" == contactInfo.m_bodyA->GetName();
	bool BisMainPlayer		= "MainPlayer" == contactInfo.m_bodyB->GetName();
	bool AisGround			= "Ground" == contactInfo.m_bodyA->GetName();
	bool BisGround			= "Ground" == contactInfo.m_bodyB->GetName();

	if (AisBullet && BisBulletDestroy)
	{
		if (!contactInfo.m_bodyA->IsDestroyed())
		{
			contactInfo.m_bodyA->Destroy();
			m_bulletEntity.reset();
		}
	}
	else if (BisBullet && AisBulletDestroy) 
	{ 
		if (!contactInfo.m_bodyB->IsDestroyed())
		{
			contactInfo.m_bodyB->Destroy();
			m_bulletEntity.reset();
		}
	}
	else if ((AisBullet && BisMainPlayer) || (BisBullet && AisMainPlayer))
	{
		RestartGame();
	}
	else if ((AisGround && BisMainPlayer) || (BisGround && AisMainPlayer))
	{
		m_isPlayerOnGround = true;
	}
}

void GameScene::ContactExitCallback(const ContactInfo & contactInfo)
{
	bool AisMainPlayer	= "MainPlayer" == contactInfo.m_bodyA->GetName();
	bool BisMainPlayer	= "MainPlayer" == contactInfo.m_bodyB->GetName();
	bool AisGround		= "Ground" == contactInfo.m_bodyA->GetName();
	bool BisGround		= "Ground" == contactInfo.m_bodyB->GetName();

	if ((AisGround && BisMainPlayer) || (BisGround && AisMainPlayer))
	{
		m_isPlayerOnGround = false;
	}
}

void GameScene::StartGame()
{
	m_physicsMgr->AddBox(math::vec2(0.f, -100.f),	math::vec2(2400.f, 20.f),	0.f,	PhysicsManager::BodyType::STATIC)->SetName("Ground");
	m_physicsMgr->AddBox(math::vec2(700.f, -100.f), math::vec2(20.f, 400.f),	0.f,	PhysicsManager::BodyType::STATIC)->SetName("BulletDestroy");
	
	m_mainEntity = m_physicsMgr->AddBox(math::vec2(500.f, -50.f), math::vec2(50.f, 50.f), 0.f, PhysicsManager::BodyType::DYNAMIC);
	m_mainEntity->SetName("MainPlayer");
	m_mainEntity->SetDensity(4.5f);
	m_mainEntity->SetGravityScale(2.f);

	m_gameRestarting	= false;
	m_isPlayerOnGround	= false;
}

void GameScene::RestartGame()
{
	m_gameRestarting = true;
}

void GameScene::EndGame()
{
	m_physicsMgr->Clear();
	m_bulletEntity.reset();
}
