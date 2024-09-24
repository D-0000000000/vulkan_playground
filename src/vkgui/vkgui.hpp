#ifndef _VKGUI_HPP_
#define _VKGUI_HPP_

#include "vkdev/vkdev.hpp"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/imgui.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <fstream>
#include <memory>

// struct UniformBufferObject
// {
// 	alignas(16) glm::mat4 model;
// 	alignas(16) glm::mat4 view;
// 	alignas(16) glm::mat4 proj;
// };

class HVKGUI
{
public:
	void setContext(std::shared_ptr<HVKContext> dev)
	{
		context = dev;
	}

	// void initVulkan()
	// {
	// 	createDescriptorSetLayout();
	// 	createGraphicsPipeline();
	// 	// createVertexBuffer();
	// 	// createIndexBuffer();
	// 	// createUniformBuffers();
	// 	createDescriptorPool();
	// 	createDescriptorSets();
	// }

	// void cleanVulkan()
	// {
	// 	context->getDevice().destroyPipeline(graphicsPipeline);
	// 	context->getDevice().destroyBuffer(vertexBuffer);
	// 	context->getDevice().destroyBuffer(indexBuffer);
	// 	context->getDevice().freeMemory(vertexBufferMemory);
	// 	context->getDevice().freeMemory(indexBufferMemory);
	// 	context->getDevice().destroyDescriptorPool(descriptorPool);
	// 	context->getDevice().destroyDescriptorSetLayout(descriptorSetLayout);
	// }

	void initImGUI();

	void init()
	{
		createDescriptorPool();
		initImGUI();
		// initVulkan();
	}

	void deinit()
	{
		ImGui_ImplVulkan_Shutdown();
		context->getDevice().destroyDescriptorPool(descriptorPool);
		// cleanVulkan();
	}

	void drawFrame();

private:
	std::shared_ptr<HVKContext> context;
	vk::PipelineCache pipelineCache;
	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::RenderPass renderPass;

	vk::DescriptorSetLayout descriptorSetLayout;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;
	vk::Buffer indexBuffer;
	vk::DeviceMemory indexBufferMemory;
	uint32_t prevVertexBufferSize = 0;
	uint32_t prevIndexBufferSize = 0;

	std::vector<vk::Buffer> uniformBuffers;
	std::vector<vk::DeviceMemory> uniformBuffersMemory;
	std::vector<void *> uniformBuffersMapped;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// ImGuiIO io;

	// void createDescriptorSetLayout();

	// void createGraphicsPipeline();

	// void createVertexBuffer();

	// void createIndexBuffer();

	// void createUniformBuffers();

	void createDescriptorPool();

	// void createDescriptorSets();

	// void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

	// void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

	// void updateUniformBuffer(uint32_t currentImage);

	// bool updateBuffers();

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
