#pragma once

#include "FANN/fann.h"

#include <vector>
#include <memory>

class ANNTrainer;

class ANNWrapper
{
public:
	struct ANNConfig
	{
		int			m_numInputs;
		int			m_numOutputs;
		int			m_numLayers;
		int			m_numNeuronsInHidden;
		float		m_maxErr;
		unsigned	m_maxEpochs;
		unsigned	m_epochsBtwnReports;
	};

	ANNWrapper(const ANNConfig& config);
	~ANNWrapper();

	std::vector<fann_type> Run(std::vector<fann_type> inputs);
	template<unsigned N> std::vector<fann_type> Run(const fann_type (&inputs)[N]);
	void Train();

	void SetTrainer(std::unique_ptr<ANNTrainer>& trainerData);
	void SetTrainer(std::unique_ptr<ANNTrainer>&& trainerData);
	ANNTrainer& GetTrainer();

	struct TrainingData
	{
		std::vector<std::vector<fann_type>> m_inputs;
		std::vector<std::vector<fann_type>> m_outputs;
		void AddData()
		{
			m_inputs.push_back({});
			m_outputs.push_back({});
		}
	};
	void TrainFromData(TrainingData * trainingData);

private:
	using InputVec = std::vector<fann_type>;
	using OutputVec = std::vector<fann_type>;
	using DataPair = std::pair<InputVec, OutputVec>;

	void PreprocessData(std::vector<std::vector<fann_type>>& inputs, std::vector<std::vector<fann_type>>& outputs) const;
	void RemoveSimilarData(std::vector<std::pair<InputVec, OutputVec>>& sortedData) const;
	void ShuffleData(std::vector<std::pair<InputVec, OutputVec>>& sortedData) const;

	friend class ANNTrainer;
	struct fann * m_ann;
	ANNConfig m_config;
	std::unique_ptr<ANNTrainer> m_annTrainer;
};

template<unsigned N>
std::vector<fann_type> ANNWrapper::Run(const fann_type(&inputs)[N])
{
	return Run(std::vector<fann_type>(inputs, inputs + N));
}