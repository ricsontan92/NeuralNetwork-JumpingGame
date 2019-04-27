#pragma once

#include <random>
#include <iostream>

class Randomizer
{
public:
	Randomizer(float minValue, float maxValue);
	Randomizer(int minValue, int maxValue);

	int GetRandomInt();
	float GetRandomFloat();

private:
	std::random_device						m_randDevice;  
	std::mt19937							m_rndGen; 
	std::uniform_int_distribution<int>		m_distributionInt;
	std::uniform_real_distribution<float>	m_distributionFloat;
};

inline Randomizer::Randomizer(float minValue, float maxValue) :	m_randDevice(),
																m_rndGen(m_randDevice()),
																m_distributionInt((int)minValue, (int)maxValue),
																m_distributionFloat(minValue, maxValue)
{
}

inline Randomizer::Randomizer(int minValue, int maxValue) : m_randDevice(),
															m_rndGen(m_randDevice()),
															m_distributionInt(minValue, maxValue),
															m_distributionFloat((float)minValue, (float)maxValue)
{
}

inline int Randomizer::GetRandomInt()
{
	return m_distributionInt(m_rndGen);
}

inline float Randomizer::GetRandomFloat()
{
	return m_distributionFloat(m_rndGen);
}