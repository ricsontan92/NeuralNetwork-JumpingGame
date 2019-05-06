#include "TrainingScene.h"

#include "JumpData.h"
#include "ANNTrainer.h"
#include "ANNWrapper.h"
#include "PhysicsBody.h"
#include "PhysicsManager.h"
#include "SceneConstants.h"
#include "GraphicsManager.h"
#include "PhysicsContactListener.h"

#include <mutex>
#include <ctime>
#include <iostream>

TrainingScene::TrainingScene(GraphicsManager& graphicsMgr, unsigned maxTrainingCount) :
	m_physicsMgr(std::make_unique<PhysicsManager>(math::vec2(0.f, -9.8f), graphicsMgr.GetDebugDrawer())),
	m_gameRestarting(false),
	m_isPlayerOnGround(false),
	m_randomizer(0.f, 2.0f),
	m_bulletSpdRandomizer(BULLET_MIN_SPEED, BULLET_MAX_SPEED),
	m_maxTrainingCount(maxTrainingCount),
	m_normalSpd(false),
	m_isRunning(true),
	m_graphicsMgr(graphicsMgr)
{
	m_physicsMgr->GetContactListener().SetBeginContactCallbackFunction(&TrainingScene::ContactEnterCallback, this);
	m_physicsMgr->GetContactListener().SetEndContactCallbackFunction(&TrainingScene::ContactExitCallback, this);

	m_annTrainer = std::make_unique<ANNTrainer>();

	m_characterTextureInfo.m_textureName = "Assets/running_anim.png";
	m_characterTextureInfo.m_cols = 8.f;
	m_characterTextureInfo.m_rows = 2.f;
	m_characterTextureInfo.m_currFrame = 0;
	m_characterTextureInfo.m_tint = math::vec4(1.f, 1.f, 1.f, 1.f);

	m_groundTextureInfo.m_textureName = "Assets/running_track.png";
	m_groundTextureInfo.m_cols = 1.f;
	m_groundTextureInfo.m_rows = 1.f;
	m_groundTextureInfo.m_currFrame = 0;
	m_groundTextureInfo.m_tint = math::vec4(1.f, 1.f, 1.f, 1.f);

	m_bulletTextureInfo.m_textureName = "Assets/missile.png";
	m_bulletTextureInfo.m_cols = 1.f;
	m_bulletTextureInfo.m_rows = 1.f;
	m_bulletTextureInfo.m_currFrame = 0;
	m_bulletTextureInfo.m_tint = math::vec4(1.f, 1.f, 1.f, 1.f);

	StartGame();
}

TrainingScene::~TrainingScene()
{
	m_physicsMgr->GetContactListener().ClearListenerFunctions();
	EndGame();
}

void TrainingScene::Update(float dt)
{
	auto time_start = std::chrono::high_resolution_clock::now();

	if ((m_animTimer += dt) >= 0.05f)
	{
		if (m_isPlayerOnGround)
			m_characterTextureInfo.m_currFrame = fmodf(m_characterTextureInfo.m_currFrame + 1.f, round(m_characterTextureInfo.m_cols * m_characterTextureInfo.m_rows * 0.5f));
		else
			m_characterTextureInfo.m_currFrame = 2.f;
		m_animTimer = 0;
	}

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

	if (m_normalSpd)
	{
		double currDt;
		do
		{
			std::this_thread::sleep_for(std::chrono::duration<double, std::nano>(1));
			currDt = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time_start).count();
		} while (currDt < dt);
	}
}

void TrainingScene::Render() const
{
	if (m_groundEntity)
	{
		m_graphicsMgr.GetRenderer().AddTextureToScene(m_groundTextureInfo,
			m_groundEntity->GetPosition() + math::vec2(0.f, 15.f),
			math::vec2(m_groundEntity->GetScale().x, 150.f),
			0.f);
	}

	if (m_mainEntity)
	{
		m_graphicsMgr.GetRenderer().AddTextureToScene(m_characterTextureInfo,
			m_mainEntity->GetPosition() - math::vec2(0.f, 0.f),
			math::vec2(-m_mainEntity->GetScale().x, m_mainEntity->GetScale().y + 15.f),
			0.f);
	}

	{
		std::lock_guard<std::mutex> lock(m_bulletMutex);
		if (m_bulletEntity)
		{
			m_graphicsMgr.GetRenderer().AddTextureToScene(m_bulletTextureInfo,
				m_bulletEntity->GetPosition(),
				math::vec2(-m_bulletEntity->GetScale().x, m_bulletEntity->GetScale().y),
				0.f);
		}

		if (m_secondBulletEntity)
		{
			m_graphicsMgr.GetRenderer().AddTextureToScene(m_bulletTextureInfo,
				m_secondBulletEntity->GetPosition(),
				math::vec2(-m_secondBulletEntity->GetScale().x, m_secondBulletEntity->GetScale().y),
				0.f);
		}
	}
	m_physicsMgr->RenderDebugShapes();
}

ANNTrainer& TrainingScene::GetANNTrainer() const
{
	return *m_annTrainer;
}

bool TrainingScene::IsTraining() const
{
	return m_isRunning && m_annTrainer->GetDataCount() < m_maxTrainingCount;
}

bool TrainingScene::IsRunning() const
{
	return m_isRunning;
}

PhysicsManager & TrainingScene::GetPhysicsManager() const
{
	return *m_physicsMgr;
}

void TrainingScene::SetNormalSpeed(bool set)
{
	m_normalSpd = set;
}

bool TrainingScene::GetNormalSpeed() const
{
	return m_normalSpd;
}

void TrainingScene::StopScene()
{
	m_isRunning = false;
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
		m_mainEntity->SetVelocity(math::vec2(0.f, PLAYER_JUMP_FORCE));

		m_isPlayerOnGround = false;
		m_jumpTimer = m_randomizer.GetRandomFloat();
		m_jumpData = std::make_unique<JumpData>();
		m_jumpData->m_inputs[(unsigned)JumpData::INPUT_TYPE::X_DIST]	= m_mainEntity->GetPosition().x - m_bulletEntity->GetPosition().x;
		m_jumpData->m_inputs[(unsigned)JumpData::INPUT_TYPE::X_SPEED]	= m_bulletEntity->GetVelocity().x;
		m_jumpData->m_inputs[(unsigned)JumpData::INPUT_TYPE::Y_DIST]	= m_bulletEntity->GetPosition().y;
	}
}

void TrainingScene::UpdateBulletEntity(float dt)
{
	if (m_isPlayerOnGround && m_bulletEntity == nullptr && m_secondBulletEntity == nullptr)
	{
		const float spawn_dist_from_player = 1200.f;
		float speed = m_bulletSpdRandomizer.GetRandomFloat();

		if ((rand() % 2) == 0)
		{
			std::lock_guard<std::mutex> lock(m_bulletMutex);
			m_secondBulletEntity = m_physicsMgr->AddBox(math::vec2(	m_mainEntity->GetPosition().x - spawn_dist_from_player - 1000.f, rand() % 2 == 0 ? 50.f : -50.f),
																	math::vec2(BULLET_WIDTH, BULLET_HEIGHT), 0.f,
																	PhysicsManager::BodyType::DYNAMIC);
			m_secondBulletEntity->SetGravityScale(0.f);
			m_secondBulletEntity->AddForceToCenter(math::vec2(speed, 0.f));
			m_secondBulletEntity->SetName("Bullet");
		}

		if (rand() % 10 != 0)	
		{
			std::lock_guard<std::mutex> lock(m_bulletMutex);
			m_bulletEntity = m_physicsMgr->AddBox(	math::vec2(m_mainEntity->GetPosition().x - spawn_dist_from_player, -50.f),
													math::vec2(BULLET_WIDTH, BULLET_HEIGHT), 0.f,
													PhysicsManager::BodyType::DYNAMIC);
		}
		else // 10% chance
		{ 
			std::lock_guard<std::mutex> lock(m_bulletMutex);
			m_bulletEntity = m_physicsMgr->AddBox(	math::vec2(m_mainEntity->GetPosition().x - spawn_dist_from_player, 50.f),
													math::vec2(BULLET_WIDTH, BULLET_HEIGHT), 0.f,
													PhysicsManager::BodyType::DYNAMIC);
		}
		m_bulletEntity->SetGravityScale(0.f);
		m_bulletEntity->AddForceToCenter(math::vec2(speed, 0.f));
		m_bulletEntity->SetName("Bullet");
	}
}

void TrainingScene::ContactEnterCallback(const ContactInfo & contactInfo)
{
	bool AisBullet			= "Bullet"			== contactInfo.m_bodyA->GetName();
	bool BisBullet			= "Bullet"			== contactInfo.m_bodyB->GetName();
	bool AisBulletDestroy	= "BulletDestroy"	== contactInfo.m_bodyA->GetName();
	bool BisBulletDestroy	= "BulletDestroy"	== contactInfo.m_bodyB->GetName();
	bool AisMainPlayer		= "MainPlayer"		== contactInfo.m_bodyA->GetName();
	bool BisMainPlayer		= "MainPlayer"		== contactInfo.m_bodyB->GetName();
	bool AisGround			= "Ground"			== contactInfo.m_bodyA->GetName();
	bool BisGround			= "Ground"			== contactInfo.m_bodyB->GetName();

	if (AisBullet && BisBulletDestroy)
	{
		if (!contactInfo.m_bodyA->IsDestroyed())
		{
			contactInfo.m_bodyA->Destroy();
			std::lock_guard<std::mutex> lock(m_bulletMutex);
			m_bulletEntity.reset();
			if (m_secondBulletEntity) m_bulletEntity = m_secondBulletEntity;
			m_secondBulletEntity.reset();
		}
	}
	else if (BisBullet && AisBulletDestroy)
	{
		if (!contactInfo.m_bodyB->IsDestroyed())
		{
			contactInfo.m_bodyB->Destroy();
			std::lock_guard<std::mutex> lock(m_bulletMutex);
			m_bulletEntity.reset();
			if (m_secondBulletEntity) m_bulletEntity = m_secondBulletEntity;
			m_secondBulletEntity.reset();
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
	m_groundEntity = m_physicsMgr->AddBox(math::vec2(0.f, -100.f), math::vec2(1800.f, 20.f), 0.f, PhysicsManager::BodyType::STATIC);
	m_groundEntity->SetName("Ground");

	m_physicsMgr->AddBox(math::vec2(700.f, -100.f), math::vec2(20.f, 400.f), 0.f, PhysicsManager::BodyType::STATIC)->SetName("BulletDestroy");

	m_mainEntity = m_physicsMgr->AddBox(math::vec2(500.f, -50.f), math::vec2(75.f, 85.f), 0.f, PhysicsManager::BodyType::DYNAMIC);
	m_mainEntity->SetName("MainPlayer");
	m_mainEntity->SetDensity(4.f);
	m_mainEntity->SetGravityScale(4.5f);
	m_mainEntity->LockRotation(true);
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

	std::lock_guard<std::mutex> lock(m_bulletMutex);
	m_bulletEntity.reset();
	m_secondBulletEntity.reset();
}