#ifndef _LASER_HPP_
#define _LASER_HPP_

#include "vkcommon/vkcommon.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

// struct pointcloud
// {
//     uint64_t timestamp;
//     double X;
//     double Y;
//     double Z;
//     uint8_t Reflectivity;
//     uint64_t Ori_x;
//     uint64_t Ori_y;
//     uint64_t Ori_z;
// };

// struct pcvk
// {
//     glm::vec3 pos;
//     glm::vec3 ref;

//     static VkVertexInputBindingDescription getBindingDescription()
//     {
//         VkVertexInputBindingDescription bindingDescription{};
//         bindingDescription.binding = 0;
//         bindingDescription.stride = sizeof(pcvk);
//         bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//         return bindingDescription;
//     }
// };

int read_laser_csv(char *filename, std::vector<Vertex> &spcvk, std::vector<uint32_t> &ind);
// int read_laser_csv(std::vector<pointcloud> pc);

#endif
