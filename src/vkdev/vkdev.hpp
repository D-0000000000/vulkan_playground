#ifndef _VKDEV_HPP_
#define _VKDEV_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <memory>

#include "vkcommon/vkcommon.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = true;
#endif

const int MAX_FRAMES_IN_FLIGHT = 2;

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

class HVKContext
{
public:
	SwapChainSupportDetails querySwapChainSupport();

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory);

	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);

	vk::CommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t heigth);

	void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

	void generateMipmaps(vk::Image image, vk::Format format, int32_t width, int32_t height, uint32_t mipLevels);

	bool isClosed();

	void mainLoopBegin()
	{
		glfwPollEvents();
		return;
	}

	void mainLoopExit()
	{
		device.waitIdle();
		return;
	}

	void buildCommandBuffer();

	void drawStart()
	{
		device.waitForFences(inFlightFences[currentFrame], vk::True, UINT64_MAX);

		vk::Result result = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			recreateSwapChain();
			return;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		device.resetFences(inFlightFences[currentFrame]);

		commandBuffers[currentFrame].reset(static_cast<vk::CommandBufferResetFlagBits>(0));

		buildCommandBuffer();

		return;
	}

	void drawEnd()
	{
		commandBuffers[currentFrame].endRenderPass();
		commandBuffers[currentFrame].end();

		vk::SubmitInfo submitInfo = vk::SubmitInfo();

		vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		submitInfo.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(waitSemaphores)
			.setWaitDstStageMask(waitStages)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&commandBuffers[currentFrame]);

		vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(signalSemaphores);

		if (graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR();
		presentInfo.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(signalSemaphores);

		vk::SwapchainKHR swapChains[] = {swapChain};
		presentInfo.setSwapchainCount(1)
			.setPSwapchains(swapChains)
			.setPImageIndices(&imageIndex);

		vk::Result result = presentQueue.presentKHR(presentInfo);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
		{
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		return;
	}

	void initWindow();

	void init();

	void deinit();

	vk::Instance getInstance()
	{
		return instance;
	}

	vk::PhysicalDevice getPhysicalDevice()
	{
		return physicalDevice;
	}

	vk::Device getDevice()
	{
		return device;
	}

	vk::SurfaceKHR getSurface()
	{
		return surface;
	}

	GLFWwindow *getWindow()
	{
		return window;
	}

	uint32_t getGraphicsQueueFamily()
	{
		return indices.graphicsFamily.value();
	}

	vk::Queue getGraphicsQueue()
	{
		return graphicsQueue;
	}

	vk::Queue getPresentQueue()
	{
		return presentQueue;
	}

	vk::Extent2D getSwapChainExtent()
	{
		return swapChainExtent;
	}

	vk::RenderPass getRenderPass()
	{
		return renderPass;
	}

	uint32_t getCurrentFrame()
	{
		return currentFrame;
	}

	uint32_t getImageIndex()
	{
		return imageIndex;
	}

	vk::CommandBuffer getCurrentCommandBuffer()
	{
		return commandBuffers[currentFrame];
	}

	vk::SampleCountFlagBits getSampleCount()
	{
		return msaaSamples;
	}

private:
	vk::PhysicalDevice physicalDevice;
	vk::Device device;
	vk::Instance instance;
	vk::SurfaceKHR surface;
	GLFWwindow *window;
	bool framebufferResized;
	VkDebugUtilsMessengerEXT debugMessenger;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	QueueFamilyIndices indices;
	uint32_t imageIndex;

	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;
	std::vector<vk::ImageView> swapChainImageViews;
	std::vector<vk::Framebuffer> swapChainFramebuffers;

	vk::Image colorImage;
	vk::DeviceMemory colorImageMemory;
	vk::ImageView colorImageView;

	vk::Image depthImage;
	vk::DeviceMemory depthImageMemory;
	vk::ImageView depthImageView;

	vk::RenderPass renderPass;

	std::vector<vk::CommandBuffer> commandBuffers;
	vk::CommandPool commandPool;

	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;
	uint32_t currentFrame = 0;

	vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

	std::vector<const char *> deviceExtensions;

	std::vector<const char *> validationLayers;

	std::vector<const char *> enabled_extensions{};

	bool checkValidationLayerSupport();

	std::vector<const char *> getRequiredExtensions();

	vk::SampleCountFlagBits getMaxUsableSampleCount();

	// QueueFamilyIndex findGraphicsQueueFamily(vk::PhysicalDevice device);

	// QueueFamilyIndex findPresentQueueFamily(vk::PhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

	QueueFamilyIndices findQueueFamilies();

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

	vk::Format findSupportedFormat(const std::vector<vk::Format> &&candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format findDepthFormat();

	bool isDeviceSuitable(vk::PhysicalDevice device);

	void pickPhysicalDevice();

	void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

	void setupDebugMessenger();

	void createInstance();

	void createSurface();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	void createColorResources();

	void createDepthResources();

	void createFramebuffers();

	void recreateSwapChain();

	void cleanupSwapChain();

	void createRenderPass();

	void createSyncObjects();

	void createCommandBuffers();

	void createCommandPool();

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		auto app = reinterpret_cast<HVKContext *>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

#endif
