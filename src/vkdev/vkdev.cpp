#include "vkdev.hpp"
#include "vkcommon/vkcommon.hpp"
#include <GLFW/glfw3.h>
#include <set>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

bool HVKPhyDev::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<vk::LayerProperties> availableLayers(layerCount);
	vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<const char *> HVKPhyDev::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void HVKPhyDev::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = vk::DebugUtilsMessengerCreateInfoEXT();
	createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback(debugCallback);
}

void HVKPhyDev::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void HVKPhyDev::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	vk::ApplicationInfo appInfo = vk::ApplicationInfo();
	appInfo.setPApplicationName("LASVK")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("No Engine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_3);

	vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo();
	createInfo.setPApplicationInfo(&appInfo);

	auto extensions = getRequiredExtensions();
	createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data());

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
			.setPpEnabledLayerNames(validationLayers.data());

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.setPNext(&debugCreateInfo);
	}
	else
	{
		createInfo.setEnabledLayerCount(0)
			.setPNext(nullptr);
	}

	if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void HVKPhyDev::createSurface()
{
	VkSurfaceKHR sf;
	if (glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &sf) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::SurfaceKHR(sf);
}

void HVKPhyDev::createLogicalDevice()
{
	indices = findQueueFamilies(physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo();
		queueCreateInfo.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1)
			.setQueuePriorities(queuePriority);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();

	vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo();

	createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPQueueCreateInfos(queueCreateInfos.data())
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
		.setPpEnabledExtensionNames(deviceExtensions.data());

	if (enableValidationLayers)
	{
		createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
			.setPpEnabledLayerNames(validationLayers.data());
	}
	else
	{
		createInfo.setEnabledLayerCount(0);
	}

	if (physicalDevice.createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
	presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

// QueueFamilyIndex HVKPhyDev::findGraphicsQueueFamily(vk::PhysicalDevice device)
// {
// 	QueueFamilyIndex index;
// 	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
// 	int i = 0;
// 	for (const auto &queueFamily : queueFamilies)
// 	{
// 		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
// 		{
// 			index = i;
// 			break;
// 		}
// 		i++;
// 	}
// 	return index;
// }

// QueueFamilyIndex HVKPhyDev::findPresentQueueFamily(vk::PhysicalDevice device)
// {
// 	QueueFamilyIndex index;
// 	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
// 	int i = 0;
// 	for (const auto &queueFamily : queueFamilies)
// 	{

// 		VkBool32 presentSupport = false;
// 		device.getSurfaceSupportKHR(i, surface, &presentSupport);

// 		if (presentSupport)
// 		{
// 			index = i;
// 			break;
// 		}

// 		i++;
// 	}

// 	return index;
// }

QueueFamilyIndices HVKPhyDev::findQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;
	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
	int i = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		vk::Bool32 presentSupport = false;
		presentSupport = device.getSurfaceSupportKHR(i, surface);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

QueueFamilyIndices HVKPhyDev::findQueueFamilies()
{
	return findQueueFamilies(physicalDevice);
}

SwapChainSupportDetails HVKPhyDev::querySwapChainSupport(vk::PhysicalDevice device)
{
	SwapChainSupportDetails details;

	details.capabilities = device.getSurfaceCapabilitiesKHR(surface);

	uint32_t formatCount;
	device.getSurfaceFormatsKHR(surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		device.getSurfaceFormatsKHR(surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	device.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		device.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

SwapChainSupportDetails HVKPhyDev::querySwapChainSupport()
{
	return querySwapChainSupport(physicalDevice);
}

bool HVKPhyDev::isDeviceSuitable(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool HVKPhyDev::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
	uint32_t extensionCount;
	device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
	device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto &extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void HVKPhyDev::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	instance.enumeratePhysicalDevices(&deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<vk::PhysicalDevice> devices(deviceCount);
	instance.enumeratePhysicalDevices(&deviceCount, devices.data());

	for (const auto &device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (!physicalDevice)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void HVKPhyDev::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

	window = glfwCreateWindow(960, 540, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	keyinput_init(window, 60);
}
