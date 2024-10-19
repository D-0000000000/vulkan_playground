#ifndef _VKCAM_HPP_
#define _VKCAM_HPP_

#include "vkdev/vkdev.hpp"

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct HVKCameraObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class HVKCamera
{
public:
	void setContext(std::shared_ptr<HVKContext> dev)
	{
		context = dev;
	}

	void init();

	void deinit();

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

	void createCamObjectBuffers();

	void createDescriptorSetLayout();

	void createDescriptorSets();

	void createDescriptorPool();

	void updateCamObjectBuffers(uint32_t currentImage);

	vk::DescriptorSetLayout getCameraDescSetLayout();

	vk::DescriptorSet getCameraDescSet(uint32_t currentImage);

private:
	std::shared_ptr<HVKContext> context;
	HVKCameraObject camObject;
	std::vector<vk::Buffer> camObjectBuffers;
	std::vector<vk::DeviceMemory> camObjectBuffersMemory;
	std::vector<void *> camObjectBuffersMapped;

	vk::DescriptorSetLayout camObjDescSetLayout;
	vk::DescriptorPool camObjDescPool;
	std::vector<vk::DescriptorSet> camObjDescSets;
};

#endif
