
#include <GLFW/glfw3.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>

static bool onExit = false;
std::thread keyinput_thread;
glm::vec3 curdir;
glm::vec3 curpos;
glm::vec3 curup;

void keyinput_main(GLFWwindow *window, uint32_t frame_rate)
{
	int state[12];
	int keystat[] = {
		GLFW_KEY_E,
		GLFW_KEY_Q,
		GLFW_KEY_S,
		GLFW_KEY_W,
		GLFW_KEY_A,
		GLFW_KEY_D};
	int keymov[] = {
		GLFW_KEY_UP,
		GLFW_KEY_DOWN,
		GLFW_KEY_LEFT,
		GLFW_KEY_RIGHT,
		GLFW_KEY_LEFT_SHIFT,
		GLFW_KEY_LEFT_CONTROL};
	curpos = glm::vec3(0.0f, 0.0f, 0.0f);
	curdir = glm::vec3(1.0f, 0.0f, 0.0f);
	curup = glm::vec3(0.0f, 0.0f, 1.0f);
	auto tt = std::chrono::steady_clock::now();
	while (!onExit)
	{
		for (int i = 0; i < 6; i++)
		{
			state[i] = glfwGetKey(window, keystat[i]);
		}
		for (int i = 0; i < 6; i++)
		{
			state[i + 6] = glfwGetKey(window, keymov[i]);
		}
		for (int i = 0; i < 3; i++)
		{
			auto vecy = glm::cross(curup, curdir);
			if (state[6 + (i << 1)] != state[6 + (i << 1 | 1)])
			{
				float offset[3] = {0.0f, 0.0f, 0.0f};
				offset[i] = ((state[6 + (i << 1)] == GLFW_PRESS) << 1) - 1.0f;
				offset[i] *= 0.025;
				curpos += offset[0] * curdir + offset[1] * vecy + offset[2] * curup;
			}
		}
		for (int i = 0; i < 3; i++)
		{
			auto vecy = glm::cross(curup, curdir);
			if (state[i << 1] != state[i << 1 | 1])
			{
				float axis[3] = {0.0f, 0.0f, 0.0f};
				axis[i] = ((state[i << 1] == GLFW_PRESS) << 1) - 1.0f;
				auto trans = glm::rotate(glm::mat4(1.0f), glm::radians(0.3f), axis[0] * curdir + axis[1] * vecy + axis[2] * curup);
				auto res = trans * glm::vec4(curdir.x, curdir.y, curdir.z, 1.0f);
				curdir.x = res.x;
				curdir.y = res.y;
				curdir.z = res.z;
				res = trans * glm::vec4(curup.x, curup.y, curup.z, 1.0f);
				curup.x = res.x;
				curup.y = res.y;
				curup.z = res.z;
			}
		}
		tt += std::chrono::nanoseconds((int)(1000000000.0 / frame_rate));
		std::this_thread::sleep_until(tt);
	}
	return;
}

void get_cam_params(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up)
{
	eye = curpos;
	center = curpos + curdir;
	up = curup;
	return;
}

void keyinput_init(GLFWwindow *window, uint32_t frame_rate)
{
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	keyinput_thread = std::thread(keyinput_main, window, frame_rate);
	return;
}

void keyinput_deinit()
{
	onExit = true;
	keyinput_thread.join();
	return;
}