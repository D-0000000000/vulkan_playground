#ifndef _VULKAN_RUN_HPP_
#define _VULKAN_RUN_HPP_

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

#include "vkcam/vkcam.hpp"
#include "vkdev/vkdev.hpp"
#include "vkgui/vkgui.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HVKApp
{
public:
	void init();

	void deinit();

	void setPrimitiveTopology(vk::PrimitiveTopology top);

	void drawFrame();

	void setIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind);

	void updateIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind);

	void setContext(std::shared_ptr<HVKContext> dev, std::shared_ptr<HVKCamera> cam, std::shared_ptr<HVKGUI> g)
	{
		context = dev;
		camera = cam;
		gui = g;
	}

private:
	std::shared_ptr<HVKContext> context;
	std::shared_ptr<HVKCamera> camera;
	std::shared_ptr<HVKGUI> gui;

	// vk::DescriptorSetLayout descriptorSetLayout;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;
	vk::Buffer indexBuffer;
	vk::DeviceMemory indexBufferMemory;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vk::PrimitiveTopology topology;

	void initVulkan();

	void createGraphicsPipeline();

	bool hasStencilComponent(vk::Format format);

	void createVertexBuffer();

	void createIndexBuffer();

	void updateVertexBuffer();

	void updateIndexBuffer();

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

	void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

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
};

#endif
