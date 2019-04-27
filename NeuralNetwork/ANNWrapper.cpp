#include "ANNWrapper.h"

#include "ANNTrainer.h"

#include <vector>
#include <cassert>
#include <iostream>

ANNWrapper::ANNWrapper(const ANNConfig& config) : 
	m_config(config),
	m_annTrainer(std::make_unique<ANNTrainer>())
{
	std::vector<unsigned> layers(m_config.m_numLayers, 0);
	layers.front() = m_config.m_numInputs;
	layers.back() = m_config.m_numOutputs;
	for (size_t i = 1; i < (layers.size() - 1); ++i)
		layers[i] = m_config.m_numNeuronsInHidden;

	m_ann = fann_create_standard_array(m_config.m_numLayers, &layers[0]);

	fann_set_activation_function_hidden(m_ann, FANN_SIGMOID_SYMMETRIC);
	fann_set_activation_function_output(m_ann, FANN_SIGMOID_SYMMETRIC);
}

ANNWrapper::~ANNWrapper()
{
	fann_destroy(m_ann);
}

std::vector<fann_type> ANNWrapper::Run(std::vector<fann_type> inputs)
{
	std::vector<fann_type> result;
	inputs = m_annTrainer->NormalizeInput(inputs);
	fann_type * output = fann_run(m_ann, &inputs[0]);
	for (int i = 0; i < m_config.m_numOutputs; ++i)
		result.push_back(output[i]);
	return result;
}

void ANNWrapper::Train()
{
	auto allOutputs = m_annTrainer->GetAllOutputs();
	auto allInputs	= m_annTrainer->GetAllInputs();
	int failures	= 0, successes = 0;

	std::vector<std::vector<fann_type>> successIns, successOuts;
	std::vector<std::vector<fann_type>> failureIns, failureOuts;

	for (unsigned i = 0; i < m_annTrainer->GetDataCount(); ++i)
	{
		if (allOutputs[i][0] == -1.f)
		{
			failures++;
			failureIns.push_back(allInputs[i]);
			failureOuts.push_back(allOutputs[i]);
		}
		else
		{
			successIns.push_back(allInputs[i]);
			successOuts.push_back(allOutputs[i]);
			successes++;
		}
	}

	std::cout << "Total Samples: " << m_annTrainer->GetDataCount() << std::endl;
	std::cout << "Failures: "	<< failures << std::endl;
	std::cout << "Successes: "	<< successes << std::endl;

	TrainingData data;
	data.m_inputs = std::move(allInputs);
	data.m_outputs = std::move(allOutputs);

	// make data ~50% - ~50%
	if (successes > 0 && failures > 0)
	{
		if (successes < failures)
		{
			int diff = failures - successes;
			while (diff > 0)
			{
				for (int i = 0; i < successes; ++i)
				{
					data.m_inputs.push_back(successIns[i]);
					data.m_outputs.push_back(successOuts[i]);
				}
				diff -= successes;
			}
		}
		else if (failures < successes)
		{
			int diff = successes - failures;
			while (diff > 0)
			{
				for (int i = 0; i < failures; ++i)
				{
					data.m_inputs.push_back(failureIns[i]);
					data.m_outputs.push_back(failureOuts[i]);
				}
				diff -= failures;
			}
		}
	}

	TrainFromData(&data);
}

void ANNWrapper::SetTrainer(std::unique_ptr<ANNTrainer>& trainerData)
{
	m_annTrainer = std::move(trainerData);
}

void ANNWrapper::SetTrainer(std::unique_ptr<ANNTrainer>&& trainerData)
{
	m_annTrainer = std::move(trainerData);
}

ANNTrainer& ANNWrapper::GetTrainer()
{
	return *m_annTrainer;
}

void ANNWrapper::TrainFromData(TrainingData * trainingData)
{
	if (trainingData == nullptr)
	{
		return;
	}

	assert(trainingData->m_inputs.size() == trainingData->m_outputs.size());
	
	std::vector<fann_type*>	forInput(trainingData->m_inputs.size(), nullptr),
							forOutput(trainingData->m_outputs.size(), nullptr);

	for (unsigned i = 0; i < trainingData->m_inputs.size(); ++i)
	{
		forInput[i]		= &(trainingData->m_inputs[i])[0];
		forOutput[i]	= &(trainingData->m_outputs[i])[0];
	}

	fann_train_data data;
	data.input			= &forInput[0];
	data.output			= &forOutput[0];
	data.num_data		= trainingData->m_outputs.size();
	data.num_input		= m_config.m_numInputs;
	data.num_output		= m_config.m_numOutputs;

	fann_train_on_data(m_ann, &data, m_config.m_maxEpochs, m_config.m_epochsBtwnReports, m_config.m_maxErr);
}