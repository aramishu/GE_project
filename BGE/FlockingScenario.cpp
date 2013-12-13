#include "FlockingScenario.h"
#include "Params.h"
#include "SteeringGame.h"
#include "FlockingGame.h"
#include "Content.h"
#include "Utils.h"
#include "Sphere.h"
#include "flock.h"


using namespace BGE;

FlockingScenario::FlockingScenario(void)
{
}


FlockingScenario::~FlockingScenario(void)
{
}

string FlockingScenario::Description()
{
	return string("Flocking Scenario");
}

void FlockingScenario::Initialise()
{
	FlockingGame * game = (FlockingGame *)Game::Instance();
	Params::Load("flocking");
	float range = Params::GetFloat("world_range");
	/*
	shared_ptr<GameComponent> fighter;
	//shared_ptr<SteeringController> fighterController;
	for (int i = 0; i < Params::GetFloat("num_boids"); i++)
	{
		glm::vec3 pos = RandomPosition(range);

		fighter = make_shared<GameComponent>();
		fighter->tag = "Steerable";
		fighter->position = pos;
		
		//fighterController = make_shared<SteeringController>();		
		//fighterController->position = pos;
		//fighterController->target = game->camera;
		//fighterController->TurnOffAll();
		//fighterController->TurnOn(SteeringController::behaviour_type::separation);
		//fighterController->TurnOn(SteeringController::behaviour_type::cohesion);
		//fighterController->TurnOn(SteeringController::behaviour_type::alignment);
		//fighterController->TurnOn(SteeringController::behaviour_type::wander);
		//fighterController->TurnOn(SteeringController::behaviour_type::evade);
		//fighterController->TurnOn(SteeringController::behaviour_type::sphere_constrain);
		//fighterController->TurnOn(SteeringController::behaviour_type::obstacle_avoidance);
		fighter->Attach(Content::LoadModel("adder", glm::rotate(glm::mat4(1), 180.0f, GameComponent::basisUp)));
		fighter->scale = glm::vec3(5,5, 5);
		//fighter->Attach(fighterController);
		//fighterController->Initialise();
		game->Attach(fighter);
	}*/
	
	
	shared_ptr<flock> flock1;
	flock1 = make_shared<flock>(3000);
	game->Attach(flock1);
	/*
	int numObstacles = 10;
	float dist = (range * 2) / numObstacles;
	for (float x = -range; x < range; x += dist)
	{
		for (float z = -range; z < range; z += dist)
		{
			shared_ptr<Sphere> obstacle = make_shared<Sphere>(10);
			obstacle->tag = "Obstacle";
			obstacle->position = glm::vec3(x, 0, z);
			game->Attach(obstacle);
		}
	}
	*/
	game->camera->position = glm::vec3(0, 335, 400);
}
