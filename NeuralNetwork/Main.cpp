#include "Camera.h"
#include "AppWindow.h"
#include "GameScene.h"
#include "ANNTrainer.h"
#include "GUIManager.h"
#include "DebugDrawer.h"
#include "TrainingScene.h"
#include "PhysicsManager.h"
#include "GraphicsManager.h"

#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <array>

double DISCRETE_DT = 1.0 / 60.0;

int main()
{
	srand(static_cast<unsigned>(time(NULL)));

	AppWindow mainWin;

	const unsigned	winWidth = 1280, winHeight = 720;
	if (!mainWin.Init(winWidth, winHeight))
	{
		return -1; // Initialization failed
	}

	double dt = 0;

	// init graphics managers
	GraphicsManager graphicsMgr;

	// training scene
	std::vector<std::future<ANNTrainer>> mTrainers;	
	for (unsigned i = 0; i < 4; ++i)
	{
		mTrainers.push_back(std::async(
			[&graphicsMgr]()->ANNTrainer
			{
				TrainingScene trainScene{ graphicsMgr , 15000 };
				while (trainScene.IsTraining())
				{
					trainScene.Update(static_cast<float>(DISCRETE_DT));
				}
				return trainScene.GetANNTrainer();
			}
		));
	}

	ANNTrainer combinedTrainer;
	for (auto & trainer : mTrainers)
	{
		trainer.wait();
		combinedTrainer += trainer.get();
	}

	// scene creation
	GameScene gameScene{ graphicsMgr, combinedTrainer };

	// gui creation
	GUIManager guiMgr{ gameScene, mainWin };

	while (!mainWin.ShouldClose())
	{
		auto time_start = std::chrono::high_resolution_clock::now();

		// fps measuring 
		mainWin.BeginFrame();

		// Keep running
		graphicsMgr.StartFrame();

			// set viewport
		int winWd, winHt;
		mainWin.GetFrameBufferSize(winWd, winHt);
		graphicsMgr.SetViewport(0, 0, winWd, winHt);

		// updating here
		float fdt = static_cast<float>(DISCRETE_DT);
		mainWin.Update();
		gameScene.Update(fdt);
		guiMgr.Update(fdt);

		// rendering here
		graphicsMgr.Render();
		guiMgr.Render();

		mainWin.PollEvent();
		mainWin.SwapBuffers();

		graphicsMgr.EndFrame();

		// frame rate controller
		do
		{
			std::this_thread::sleep_for(std::chrono::duration<double, std::nano>(1));
			dt = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time_start).count();
		} while (dt < DISCRETE_DT);

		mainWin.EndFrame();
	}

	return 0;
}
