#ifndef _VKDEV_HPP_
#define _VKDEV_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <memory>

#include "keyinput/keyinput.hpp"
#include "vkcommon/vkcommon.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

class HVKPhyDev
{
public:
	bool checkValidationLayerSupport();

	std::vector<const char *> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

	void setupDebugMessenger();

	void createInstance();

	void createSurface();

	void createLogicalDevice();

	// QueueFamilyIndex findGraphicsQueueFamily(vk::PhysicalDevice device);

	// QueueFamilyIndex findPresentQueueFamily(vk::PhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

	QueueFamilyIndices findQueueFamilies();

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport();

	bool isDeviceSuitable(vk::PhysicalDevice device);

	void pickPhysicalDevice();

	void initPhyDev()
	{
		pickPhysicalDevice();
	}

	void initWindow();

	void deinit()
	{
		device.destroy();

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(VkInstance(instance), debugMessenger, nullptr);
		}

		instance.destroySurfaceKHR(surface, nullptr);
		instance.destroy(nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
		keyinput_deinit();
	}

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		auto app = reinterpret_cast<HVKPhyDev *>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void init()
	{
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		initWindow();
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	std::shared_ptr<vk::Instance> getInstance()
	{
		return std::shared_ptr<vk::Instance>(&instance);
	}

	vk::PhysicalDevice getPhysicalDevice()
	{
		return physicalDevice;
	}

	vk::Device getDevice()
	{
		return device;
	}

	VkSurfaceKHR getSurface()
	{
		return surface;
	}

	GLFWwindow *getWindow()
	{
		return window;
	}

	bool checkFrameBufferResized()
	{
		return framebufferResized;
	}

	void setFrameBufferResized(bool flag)
	{
		framebufferResized = flag;
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

private:
	vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
	vk::Device device;
	vk::Instance instance;
	VkSurfaceKHR surface;
	GLFWwindow *window;
	bool framebufferResized;
	VkDebugUtilsMessengerEXT debugMessenger;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	QueueFamilyIndices indices;
	std::vector<const char *> deviceExtensions;

	std::vector<const char *> validationLayers;

	std::vector<const char *> enabled_extensions{};

	static VKAPI_ATTR VkBool32 VKAPI_CALL
	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

#endif
