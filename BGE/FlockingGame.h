#pragma once
#include "Game.h"
#include "SteeringControler.h"
#include "GameComponent.h"
#include <vector>
#include "Scenario.h"

using namespace std;

namespace BGE
{
	class FlockingGame :
		public Game
	{
	public:
		FlockingGame(void);
		~FlockingGame(void);

		bool Initialise();
		void Update(float timeDelta);
		void Reset();

		shared_ptr<GameComponent> camFollower;

		bool lastPressed;
		bool camFollowing;

		Scenario* scenario;

		float elapsed;
	};
}
