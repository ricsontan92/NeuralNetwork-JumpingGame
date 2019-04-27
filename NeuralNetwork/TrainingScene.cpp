#include "TrainingScene.h"

#include "JumpData.h"
#include "ANNTrainer.h"
#include "ANNWrapper.h"
#include "PhysicsBody.h"
#include "PhysicsManager.h"
#include "GraphicsManager.h"
#include "PhysicsContactListener.h"

#include <ctime>
#include <iostream>

TrainingScene::TrainingScene(GraphicsManager& graphicsMgr, unsigned maxTrainingCount) :
	m_physicsMgr(std::make_unique<PhysicsManager>(math::vec2(0.f, -9.8f), graphicsMgr.GetDebugDrawer())),
	m_gameRestarting(false),
	m_isPlayerOnGround(false),
	m_randomizer(0.f, 1.f),
	m_maxTrainingCount(maxTrainingCount)
{
	srand(static_cast<unsigned>(time(NULL)));

	m_physicsMgr->GetContactListener().SetBeginContactCallbackFunction(&TrainingScene::ContactEnterCallback, this);
	m_physicsMgr->GetContactListener().SetEndContactCallbackFunction(&TrainingScene::ContactExitCallback, this);

	m_annTrainer = std::make_unique<ANNTrainer>();

	StartGame();
}

TrainingScene::~TrainingScene()
{
	m_physicsMgr->GetContactListener().ClearListenerFunctions();
	EndGame();
}

void TrainingScene::Update(float dt)
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
	}
}

ANNTrainer& TrainingScene::GetANNTrainer() const
{
	return *m_annTrainer;
}

bool TrainingScene::IsTraining() const
{
	return m_annTrainer->GetDataCount() < m_maxTrainingCount;
}

void TrainingScene::UpdateMainEntity(float dt)
{
	if (!m_isPlayerOnGround)
	{
		if (m_jumpData && m_bulletEntity)
		{
			if (m_jumpData->m_inputs[(unsigned)JumpData::INPUT_TYPE::X_DIST] > 0.f &&
				m_mainEntity->GetPosition().x < m_bulletEntity->GetPosition().x &&
				m_mainEntity->GetPosition().y > m_bulletEntity->GetPosition().y)
			{
				m_jumpData->m_outputs[(unsigned)JumpData::OUTPUT_TYPE::JUMP] = 1.f;
			}
		}
		return;
	}

	if (m_bulletEntity && (m_jumpTimer -= dt) <= 0.f)
	{
		// jump
		m_mainEntity->AddForceToCenter(math::vec2(0.f, 3000.f));

		m_isPlayerOnGround = false;
		m_jumpTimer = m_randomizer.GetRandomFloat();
		m_jumpData = std::make_unique<JumpData>();
		m_jumpData->m_inputs[(unsigned)JumpData::INPUT_TYPE::X_DIST] = m_mainEntity->GetPosition().x - m_bulletEntity->GetPosition().x;
		m_jumpData->m_inputs[(unsigned)JumpData::INPUT_TYPE::X_SPEED] = m_bulletEntity->GetVelocity().x;
	}
}

void TrainingScene::UpdateBulletEntity(float dt)
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

void TrainingScene::ContactEnterCallback(const ContactInfo & contactInfo)
{
	bool AisBullet = "Bullet" == contactInfo.m_bodyA->GetName();
	bool BisBullet = "Bullet" == contactInfo.m_bodyB->GetName();
	bool AisBulletDestroy = "BulletDestroy" == contactInfo.m_bodyA->GetName();
	bool BisBulletDestroy = "BulletDestroy" == contactInfo.m_bodyB->GetName();
	bool AisMainPlayer = "MainPlayer" == contactInfo.m_bodyA->GetName();
	bool BisMainPlayer = "MainPlayer" == contactInfo.m_bodyB->GetName();
	bool AisGround = "Ground" == contactInfo.m_bodyA->GetName();
	bool BisGround = "Ground" == contactInfo.m_bodyB->GetName();

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
		if (m_jumpData)
		{
			m_jumpData->m_outputs[(unsigned)JumpData::OUTPUT_TYPE::JUMP] = -1.f;
			m_annTrainer->InputData(m_jumpData->GetInputsAsVector(), m_jumpData->GetOutputsAsVector());
			m_jumpData.reset();
		}
		RestartGame();
	}
	else if ((AisGround && BisMainPlayer) || (BisGround && AisMainPlayer))
	{
		if (m_jumpData)
		{
			if (m_jumpData->m_outputs[(unsigned)JumpData::OUTPUT_TYPE::JUMP] == 0.f)
				m_jumpData->m_outputs[(unsigned)JumpData::OUTPUT_TYPE::JUMP] = -1.f;	// didn't jump over anything

			m_annTrainer->InputData(m_jumpData->GetInputsAsVector(), m_jumpData->GetOutputsAsVector());
			m_jumpData.reset();
		}
		m_isPlayerOnGround = true;
	}
}

void TrainingScene::ContactExitCallback(const ContactInfo & contactInfo)
{
	bool AisMainPlayer = "MainPlayer" == contactInfo.m_bodyA->GetName();
	bool BisMainPlayer = "MainPlayer" == contactInfo.m_bodyB->GetName();
	bool AisGround = "Ground" == contactInfo.m_bodyA->GetName();
	bool BisGround = "Ground" == contactInfo.m_bodyB->GetName();

	if ((AisGround && BisMainPlayer) || (BisGround && AisMainPlayer))
	{
		m_isPlayerOnGround = false;
	}
}

void TrainingScene::StartGame()
{
	m_physicsMgr->AddBox(math::vec2(0.f, -100.f), math::vec2(2400.f, 20.f), 0.f, PhysicsManager::BodyType::STATIC)->SetName("Ground");
	m_physicsMgr->AddBox(math::vec2(700.f, -100.f), math::vec2(20.f, 400.f), 0.f, PhysicsManager::BodyType::STATIC)->SetName("BulletDestroy");

	m_mainEntity = m_physicsMgr->AddBox(math::vec2(500.f, -50.f), math::vec2(50.f, 50.f), 0.f, PhysicsManager::BodyType::DYNAMIC);
	m_mainEntity->SetName("MainPlayer");
	m_mainEntity->SetDensity(4.5f);
	m_mainEntity->SetGravityScale(2.f);

	m_gameRestarting = false;
	m_isPlayerOnGround = false;

	m_jumpTimer = m_randomizer.GetRandomFloat();
}

void TrainingScene::RestartGame()
{
	m_gameRestarting = true;
}

void TrainingScene::EndGame()
{
	m_physicsMgr->Clear();
	m_bulletEntity.reset();
}