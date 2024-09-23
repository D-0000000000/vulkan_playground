#ifndef _VULKAN_RUN_HPP_
#define _VULKAN_RUN_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/imgui.h"

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

#include "keyinput/keyinput.hpp"
#include "vkdev/vkdev.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
	// glm::vec2 pos;
	glm::vec3 pos;
	glm::vec3 color;

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription();
		bindingDescription.setBinding(0)
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex);

		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0] = vk::VertexInputAttributeDescription();
		attributeDescriptions[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, pos));

		attributeDescriptions[1].setBinding(0).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, color));

		return attributeDescriptions;
	}
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

extern std::vector<Vertex> vertices;
extern std::vector<uint32_t> indices;

void setIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind);

class HVKApp
{
public:
	void setPhyDev(std::shared_ptr<HVKPhyDev> dev)
	{
		phyDev = dev;
	}

	void run()
	{
		initVulkan();
		mainLoop();
		cleanup();
	}

	uint32_t getFrameCount()
	{
		return frame_count;
	}

private:
	std::shared_ptr<HVKPhyDev> phyDev;
	uint32_t frame_count;
	uint32_t frame_rate;
	float frame_time;

	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;
	std::vector<vk::ImageView> swapChainImageViews;
	std::vector<vk::Framebuffer> swapChainFramebuffers;

	vk::RenderPass renderPass;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline graphicsPipeline;

	vk::CommandPool commandPool;

	vk::Image depthImage;
	vk::DeviceMemory depthImageMemory;
	vk::ImageView depthImageView;

	vk::Buffer vertexBuffer;
	vk::DeviceMemory vertexBufferMemory;
	vk::Buffer indexBuffer;
	vk::DeviceMemory indexBufferMemory;

	std::vector<vk::Buffer> uniformBuffers;
	std::vector<vk::DeviceMemory> uniformBuffersMemory;
	std::vector<void *> uniformBuffersMapped;

	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;

	std::vector<vk::CommandBuffer> commandBuffers;

	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;
	uint32_t currentFrame = 0;

	void initVulkan();

	void mainLoop();

	void cleanupSwapChain();

	void cleanup();

	void recreateSwapChain();

	void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

	void createSwapChain();

	void createImageViews();

	void createRenderPass();

	void createDescriptorSetLayout();

	void createGraphicsPipeline();

	void createFramebuffers();

	void createCommandPool();

	vk::Format findSupportedFormat(const std::vector<vk::Format> &&candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format findDepthFormat();

	void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory);

	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

	void createDepthResources();

	bool hasStencilComponent(vk::Format format);

	void createVertexBuffer();

	void createIndexBuffer();

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	void createCommandBuffers();

	void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	vk::ShaderModule createShaderModule(const std::vector<char> &code);

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

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
