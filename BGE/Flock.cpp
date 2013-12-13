#include "flock.h"																																lude "flock.h"#include "Params.h"
#include "Content.h"
#include "Utils.h"
#include "FlockingGame.h"
#include "Params.h"

#include "CL\cl.h"
#include "CL\opencl.h"

using namespace BGE;

cl_context context;
cl_program wanderProg;
cl_kernel wanderKern;
cl_command_queue queue;
cl_uint platformIdCount = 0;
cl_uint deviceIdCount = 0;

void CheckError(cl_int error)
{
	if (error != CL_SUCCESS) {
		std::cerr <<	 " << error << std::endl;
		std::exit(1);
	}
}

cl_program CreateProgram(const std::string& source, cl_context context)
{
	size_t lengths[1] = { source.size() };
	const char* sources[1] = { source.data() };

	cl_int error = 0;
	cl_program program = clCreateProgramWithSource(context, 1, sources, lengths, &error);
	CheckError(error);
	return program;
}

std::string LoadKernel(const char* name)
{
	std::ifstream in(name);
	std::string result((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	return result;
}

flock::flock()
{
	//use a default number of boids
	flock(10);
}

flock::flock(int numBoids)
{
	//opencl setup 
	clGetPlatformIDs (0, nullptr, &platformIdCount);
	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs (platformIdCount, platformIds.data(), nullptr);
	
	//my platform (ivy bridge w/ HD4000) supports opencl on the gpu and cpu so we'll explicitly look for the gpu here. -aramishu
	clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, 0, nullptr, &deviceIdCount);
	std::vector<cl_device_id> deviceIds(deviceIdCount);
	clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, deviceIdCount, deviceIds.data(), nullptr);

	const cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		reinterpret_cast<cl_context_properties> (platformIds[0]),
		0, 0
	};
	cl_int error;

	context = clCreateContext(contextProperties, deviceIdCount, deviceIds.data(), nullptr, nullptr, &error);

	wanderProg = CreateProgram(LoadKernel("wander.cl"), context);

	clBuildProgram(wanderProg, deviceIdCount, deviceIds.data(), nullptr, nullptr, nullptr);

	wanderKern = clCreateKernel(wanderProg, "wander", &error);
	CheckError(error);

	queue = clCreateCommandQueue(context, deviceIds[0], 0, &error);

		


	//create boids
	Params::Load("flocking");
	float range = Params::GetFloat("world_range");
	FlockingGame * game = (FlockingGame *)Game::Instance();

	shared_ptr<GameComponent> fighter;

	for (int i = 0; i < numBoids; i++)
	{
		glm::vec3 pos = RandomPosition(range);

		fighter = make_shared<GameComponent>();
		fighter->wanderTarget = RandomPosition(1.0f);
		boids.push_back(fighter);

		fighter->position = pos;

		fighter->Attach(Content::LoadModel("adder", glm::rotate(glm::mat4(1), 180.0f, GameComponent::basisUp)));
		fighter->scale = glm::vec3(5, 5, 5);

		game->Attach(fighter);
	}
}

flock::~flock()
{
	clReleaseCommandQueue(queue);
	clReleaseKernel(wanderKern);
	clReleaseProgram(wanderProg);
	clReleaseContext(context);
}



void flock::Update(float timeDelta)
{
	/*	//cpu wander really basic implementation
		float jitterTimeSlice = Params::GetFloat("wander_jitter") * timeDelta;
		float wander_radius = Params::GetFloat("wander_radius");
		float wander_weight = Params::GetWeight("wander_weight");
		float mass = 1.0f;
		glm::vec3 force = glm::vec3(0);

		for (std::vector<shared_ptr<GameComponent>>::iterator boid = boids.begin(); boid != boids.end(); ++boid)
		{
		glm::vec3 toAdd = glm::vec3(RandomClamped(), RandomClamped(), RandomClamped()) * jitterTimeSlice;
		boid->get()->wanderTarget += toAdd;
		boid->get()->wanderTarget = glm::normalize(boid->get()->wanderTarget);
		boid->get()->wanderTarget *= wander_radius;

		glm::vec3 worldTarget = glm::vec3(world * glm::vec4(boid->get()->wanderTarget + (GameComponent::basisLook * Params::GetFloat("wander_distance")), 1.0f));

		glm::vec3 wander_force = (worldTarget - boid->get()->position) * wander_weight;

		force += wander_force;

		glm::vec3 acceleration = force / mass;
		boid->get()->velocity += acceleration * timeDelta;

		boid->get()->position += boid->get()->velocity * timeDelta;

		// Apply damping
		boid->get()->velocity *= 0.99f;
		}*/

	// need to write the position, and velocity vectors for the voids to the kernel input buffer in such a way the kernel can interperat it.
	//cl_mem inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float), &error); // kernel input data
	//cl_mem outBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float), &error); // kernel output data
	cl_int error;
	int numBoids = boids.size();
	size_t bytes = sizeof(float)* numBoids;
	//hoing to use arrays of floats to pass array values
	float* pos_x = new float[numBoids];
	float* pos_y = new float[numBoids];
	float* pos_z = new float[numBoids];

	float* vel_x = new float[numBoids];
	float* vel_y = new float[numBoids];
	float* vel_z = new float[numBoids];

	int pos = 0;
	//populate arrays
	for (std::vector < shared_ptr < GameComponent >> ::iterator boid = boids.begin(); boid != boids.end(); ++boid)
	{
		pos_x[pos] = boid->get()->position.x;
		pos_y[pos] = boid->get()->position.y;
		pos_z[pos] = boid->get()->position.z;

		vel_x[pos] = boid->get()->velocity.x;
		vel_y[pos] = boid->get()->velocity.y;
		vel_z[pos] = boid->get()->velocity.z;
		pos++;
	}

	cl_mem pos_x_inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, bytes, &error);
	cl_mem pos_y_inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, bytes, &error);
	cl_mem pos_z_inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, bytes, &error);

	cl_mem vel_x_inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, bytes, &error);
	cl_mem vel_y_inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, bytes, &error);
	cl_mem vel_z_inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, bytes, &error);

	clEnqueueWriteBuffer(queue, pos_x_inBuffer, CL_TRUE, 0, bytes, pos_x, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, pos_y_inBuffer, CL_TRUE, 0, bytes, pos_y, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, pos_z_inBuffer, CL_TRUE, 0, bytes, pos_z, 0, NULL, NULL);

	clEnqueueWriteBuffer(queue, vel_x_inBuffer, CL_TRUE, 0, bytes, vel_x, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, vel_y_inBuffer, CL_TRUE, 0, bytes, vel_y, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, vel_z_inBuffer, CL_TRUE, 0, bytes, vel_z, 0, NULL, NULL);

	clSetKernelArg(wanderKern, 0, sizeof(cl_mem), &pos_x_inBuffer);
	clSetKernelArg(wanderKern, 0, sizeof(cl_mem), &pos_y_inBuffer);
	clSetKernelArg(wanderKern, 0, sizeof(cl_mem), &pos_z_inBuffer);

	clSetKernelArg(wanderKern, 0, sizeof(cl_mem), &vel_x_inBuffer);
	clSetKernelArg(wanderKern, 0, sizeof(cl_mem), &vel_x_inBuffer);
	clSetKernelArg(wanderKern, 0, sizeof(cl_mem), &vel_x_inBuffer);
	
	//TODO: create output arrays and buffers

	const size_t globalWorkSize[] = { numBoids, 0, 0 };
	CheckError(clEnqueueNDRangeKernel(queue, wanderKern, 1, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr));
}

