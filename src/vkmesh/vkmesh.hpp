#ifndef _VKMESH_HPP_
#define _VKMESH_HPP_

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

#include "vkdev/vkdev.hpp"

class HVKMesh
{
public:
	void init();

	void deinit();

	void drawFrame();

	void setIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind);

	void setMeshTexturePath(std::string mPath, std::string tPath);

	void setContext(std::shared_ptr<HVKContext> dev)
	{
		context = dev;
	}

private:
	std::shared_ptr<HVKContext> context;

	vk::DescriptorSetLayout descriptorSetLayout;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;
	vk::Buffer indexBuffer;
	vk::DeviceMemory indexBufferMemory;

	uint32_t mipLevels;
	vk::Image textureImage;
	vk::DeviceMemory textureImageMemory;
	vk::ImageView textureImageView;
	vk::Sampler textureSampler;

	std::vector<vk::Buffer> uniformBuffers;
	std::vector<vk::DeviceMemory> uniformBuffersMemory;
	std::vector<void *> uniformBuffersMapped;

	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::string meshPath;
	std::string texturePath;

	void initVulkan();

	void createTextureImage();

	void createTextureImageView();

	void createTextureSampler();

	void createDescriptorSetLayout();

	void createGraphicsPipeline();

	bool hasStencilComponent(vk::Format format);

	void createVertexBuffer();

	void createIndexBuffer();

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

	void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

	void updateUniformBuffer(uint32_t currentImage);

	vk::ShaderModule createShaderModule(const std::vector<char> &code);

	static std::vector<char> readFile(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

#endif
