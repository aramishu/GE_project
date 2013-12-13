#include "gpucontroller.h"
#include "Game.h"
#include <gtx/norm.hpp>
#include <string>
#include "Utils.h"
#include "Params.h"
#include "Sphere.h"
#include <limits>

using namespace BGE;

bool GPUcontroller::counted = false;
vector<shared_ptr<GameComponent>> GPUcontroller::obstacles;
vector<shared_ptr<GameComponent>> GPUcontroller::steerables;


GPUcontroller::GPUcontroller(void)
{
	force = glm::vec3(0);
	acceleration = glm::vec3(0);
	mass = 1.0f;
	timeDelta = 0.0f;
	worldMode = world_modes::to_parent;
	calculationMethod = CalculationMethods::WeightedTruncatedRunningSumWithPrioritisation;
	maxSpeed = Params::GetFloat("max_speed");
	maxForce = Params::GetFloat("max_force");

	wanderTarget = RandomPosition(1.0f);
	wanderTarget = glm::normalize(wanderTarget);
	wanderTarget *= Params::GetFloat("wander_radius");
	counted = false;

	route = make_shared<Route>();

	// Start with all behaviours turned off
	TurnOffAll();
}

bool GPUcontroller::Initialise()
{
	if (initialised)
	{
		return true;
	}
	Attach(route);
	initialised = true;
	return GameComponent::Initialise();
}


GPUcontroller::~GPUcontroller(void)
{
}

void GPUcontroller::Update(float timeDelta)
{
	float smoothRate;
	this->timeDelta = timeDelta;

	if (!SteeringController::counted)
	{
		list<shared_ptr<GameComponent>>::iterator oit = Game::Instance()->children.begin();
		while (oit != Game::Instance()->children.end())
		{
			if ((*oit)->tag == "Obstacle")
			{
				SteeringController::obstacles.push_back(*oit);
			}
			if ((*oit)->tag == "Steerable")
			{
				SteeringController::steerables.push_back(*oit);
			}
			oit++;
		}
		SteeringController::counted = true;
	}


	force = Calculate();
	CheckNaN(force);
	glm::vec3 newAcceleration = force / mass;

	if (timeDelta > 0)
	{
		smoothRate = Clip(9 * timeDelta, 0.15f, 0.4f) / 2.0f;
		BlendIntoAccumulator(smoothRate, newAcceleration, acceleration);
	}

	velocity += acceleration * timeDelta;
	float speed = glm::length(velocity);
	if (speed > maxSpeed)
	{

		velocity = glm::normalize(velocity);
		velocity *= maxSpeed;
	}
	position += velocity * timeDelta;

	// the length of this global-upward-pointing vector controls the vehicle's
	// tendency to right itself as it is rolled over from turning acceleration
	glm::vec3 globalUp = glm::vec3(0, 0.2f, 0);
	// acceleration points toward the center of local path curvature, the
	// length determines how much the vehicle will roll while turning
	glm::vec3 accelUp = acceleration * 0.05f;
	// combined banking, sum of UP due to turning and global UP
	glm::vec3 bankUp = accelUp + globalUp;
	// blend bankUp into vehicle's UP basis vector
	smoothRate = timeDelta * 3;
	glm::vec3 tempUp = up;
	BlendIntoAccumulator(smoothRate, bankUp, tempUp);
	up = tempUp;
	up = glm::normalize(up);

	if (speed > 0.0001f)
	{
		look = velocity;
		look = glm::normalize(look);
		if (look == right)
		{
			right = GameComponent::basisRight;
		}
		else
		{
			right = glm::cross(look, up);
			glm::normalize(right);

			CheckNaN(right, GameComponent::basisRight);
			up = glm::cross(right, look);
			up = glm::normalize(up);
			CheckNaN(up, GameComponent::basisUp);
		}
		// Apply damping
		velocity *= 0.99f;
	}

	if (look != GameComponent::basisLook)
	{
		// Make a quaternion from the vectors
		glm::mat4 rotationMatrix = glm::lookAt(glm::vec3(0), look, up);
		rotationMatrix = glm::transpose(rotationMatrix);
		orientation = glm::quat(rotationMatrix);
	}

	GameComponent::Update(timeDelta);
}

bool GPUcontroller::IsOn(behaviour_type behaviour)
{
	return ((flags & (int)behaviour) == (int)behaviour);
}

void GPUcontroller::TurnOn(behaviour_type behaviour)
{
	flags |= ((int)behaviour);
}

void GPUcontroller::TurnOffAll()
{
	flags = (int)GPUcontroller::behaviour_type::none;
}

glm::vec3 GPUcontroller::RandomWalk()
{
	float dist = glm::length(position - randomWalkTarget);
	if (dist < 50)
	{
		randomWalkTarget.x = RandomClamped() * Params::GetFloat("world_range");
		randomWalkTarget.y = RandomClamped() * Params::GetFloat("world_range");
		randomWalkTarget.z = RandomClamped() * Params::GetFloat("world_range");
	}
	return Seek(randomWalkTarget);
}

glm::vec3 GPUcontroller::Wander()
{
	float jitterTimeSlice = Params::GetFloat("wander_jitter") * timeDelta;

	glm::vec3 toAdd = glm::vec3(RandomClamped(), RandomClamped(), RandomClamped()) * jitterTimeSlice;
	wanderTarget += toAdd;
	wanderTarget = glm::normalize(wanderTarget);
	wanderTarget *= Params::GetFloat("wander_radius");

	glm::vec3 worldTarget = glm::vec3(world * glm::vec4(wanderTarget + (GameComponent::basisLook * Params::GetFloat("wander_distance")), 1.0f));

	return (worldTarget - position);

}
