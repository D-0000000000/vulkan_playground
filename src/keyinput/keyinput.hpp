#ifndef _KEYINPUT_HPP_
#define _KEYINPUT_HPP_

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void get_cam_params(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up);

void keyinput_init(GLFWwindow *window, uint32_t frame_rate);

void keyinput_deinit();

void keyinput_run();

#endif
