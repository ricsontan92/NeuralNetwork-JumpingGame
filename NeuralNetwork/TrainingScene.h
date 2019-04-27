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

class TrainingScene
{
public:
	TrainingScene(GraphicsManager& graphicsMgr, unsigned maxTrainingCount);
	~TrainingScene();
	virtual void Update(float dt);
	ANNTrainer& GetANNTrainer() const;
	bool IsTraining() const;
protected:
	virtual void UpdateMainEntity(float dt);
	virtual void UpdateBulletEntity(float dt);
	virtual void ContactEnterCallback(const ContactInfo & contactInfo);
	virtual void ContactExitCallback(const ContactInfo & contactInfo);

	virtual void StartGame();
	virtual void RestartGame();
	virtual void EndGame();

	unsigned						m_maxTrainingCount;

	float							m_jumpTimer;
	bool							m_isPlayerOnGround;
	bool							m_gameRestarting;

	Randomizer						m_randomizer;

	std::unique_ptr<ANNTrainer>		m_annTrainer;
	std::unique_ptr<PhysicsManager> m_physicsMgr;
	std::unique_ptr<JumpData>		m_jumpData;

	std::shared_ptr<PhysicsBody>	m_mainEntity;
	std::shared_ptr<PhysicsBody>	m_bulletEntity;
};