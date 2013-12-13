#include "FlockingGame.h"

#include "Params.h"
#include "SteeringControler.h"
#include "Content.h"
#include "VectorDrawer.h"
#include "Scenario.h"
#include "FlockingScenario.h"
#include "PathFollowingScenario.h"
#include "ObstacleAvoidanceScenario.h"

using namespace BGE;

FlockingGame::FlockingGame(void)
{
	lastPressed = false;
	camFollowing = false;
	scenario = new FlockingScenario();

	elapsed = 10000;
}


FlockingGame::~FlockingGame(void)
{
}

bool FlockingGame::Initialise()
{
	Params::Load("default");

	width = 680;
	height = 400;
	riftEnabled = false;
	fullscreen = false;

	scenario->Initialise();

	return Game::Initialise();
}


void FlockingGame::Update(float timeDelta)
{
	static float multiplier = 1.0f;

	float timeToPass = 1.0f;

	if (keyState[SDL_SCANCODE_F1])
	{
		if (!lastPressed)
		{
			camFollowing = !camFollowing;
			lastPressed = true;
		}
	}
	else
	{
		lastPressed = false;
	}
	elapsed += timeDelta;

	PrintText("Press O to decrease speed");
	PrintText("Press P to increase speed");

	if (keyState[SDL_SCANCODE_O])
	{
		multiplier -= timeDelta;
	}
	if (keyState[SDL_SCANCODE_P])
	{
		multiplier += timeDelta;
	}
	scenario->Update(timeDelta * multiplier);

	Game::Update(timeDelta  * multiplier);

	if (camFollowing)
	{
		camera->GetController()->position = camFollower->position;
		camera->orientation = camFollower->orientation * camera->GetController()->orientation;
		camera->RecalculateVectors();
		camera->view = glm::lookAt(
			camera->position
			, camera->position + camera->look
			, camera->up
			);

	}
}
