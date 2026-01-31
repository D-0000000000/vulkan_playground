#include "simulation.hpp"

#include <semaphore>
#include <thread>

#include <glm/glm.hpp>

static bool isRunning;
static bool simStart;
static glm::vec3 position;

std::thread sim_thread;

glm::vec3 getPosition()
{
	return position;
}

void sim_main()
{
	glm::vec3 pos(1.0f, 0.0f, 0.0f);
	glm::vec3 vel(0.0f, 0.8f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);
	position = pos;

	isRunning = true;

	double gcoeff = 1;
	uint32_t step = 1;

	auto tt = std::chrono::steady_clock::now();

	while (!sim_start)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	while (isRunning)
	{
		glm::vec3 dir = center - pos;
		glm::vec3 acc = glm::normalize(dir) / glm::length(dir);
		vel = vel + acc * (step * 1e-6f);
		pos = pos + vel * (step * 1e-6f);
		position = pos;

		auto tnext = tt + std::chrono::milliseconds(step);
		std::this_thread::sleep_until(tnext);
	}

	return;
}

int sim_init()
{
	simStart = false;
	position = glm::vec3(1.0f, 0.0f, 0.0f);
	sim_thread = std::thread(sim_main);
	return 0;
}

int sim_start()
{
	simStart = true;
}

int sim_deinit()
{
	isRunning = false;
	sim_thread.join();
	return 0;
}
