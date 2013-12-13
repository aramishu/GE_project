#pragma once
#include "GameComponent.h"
#include <vector>
#include "CL\cl.h"

using namespace std;

namespace BGE
{
	class flock :
		public GameComponent
	{
	private:
		vector<shared_ptr<GameComponent>> boids;

	public:
		flock();
		flock(int);
		~flock();

		void Update(float timeDelta);
	};
}
