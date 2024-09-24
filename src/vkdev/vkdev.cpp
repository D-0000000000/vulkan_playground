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

bool HVKContext::checkValidationLayerSupport()
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

std::vector<const char *> HVKContext::getRequiredExtensions()
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

void HVKContext::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = vk::DebugUtilsMessengerCreateInfoEXT();
	createInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback(debugCallback);
}

void HVKContext::setupDebugMessenger()
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

// QueueFamilyIndex HVKContext::findGraphicsQueueFamily(vk::PhysicalDevice device)
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

// QueueFamilyIndex HVKContext::findPresentQueueFamily(vk::PhysicalDevice device)
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

QueueFamilyIndices HVKContext::findQueueFamilies(vk::PhysicalDevice device)
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

QueueFamilyIndices HVKContext::findQueueFamilies()
{
	return findQueueFamilies(physicalDevice);
}

SwapChainSupportDetails HVKContext::querySwapChainSupport(vk::PhysicalDevice device)
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

SwapChainSupportDetails HVKContext::querySwapChainSupport()
{
	return querySwapChainSupport(physicalDevice);
}

bool HVKContext::isDeviceSuitable(vk::PhysicalDevice device)
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

bool HVKContext::checkDeviceExtensionSupport(vk::PhysicalDevice device)
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

vk::SurfaceFormatKHR HVKContext::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR HVKContext::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eFifo)
		{
			return availablePresentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D HVKContext::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		vk::Extent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

uint32_t HVKContext::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

vk::ImageView HVKContext::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
	vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
	viewInfo.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setSubresourceRange(
			vk::ImageSubresourceRange()
				.setAspectMask(aspectFlags)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1));
	vk::ImageView imageView = device.createImageView(viewInfo, nullptr);
	if (!imageView)
	{
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}

vk::Format HVKContext::findSupportedFormat(const std::vector<vk::Format> &&candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties props;
		physicalDevice.getFormatProperties(format, &props);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

vk::Format HVKContext::findDepthFormat()
{
	return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void HVKContext::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory)
{
	vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();
	imageInfo.setImageType(vk::ImageType::e2D)
		.setExtent(vk::Extent3D().setWidth(width).setHeight(height).setDepth(1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(tiling)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(usage)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setSharingMode(vk::SharingMode::eExclusive);

	image = device.createImage(imageInfo, nullptr);
	if (!image)
	{
		throw std::runtime_error("failed to create image!");
	}

	vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo();
	allocInfo.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
	imageMemory = device.allocateMemory(allocInfo, nullptr);
	if (!imageMemory)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	device.bindImageMemory(image, imageMemory, 0);
}

void HVKContext::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(commandPool)
		.setCommandBufferCount(1);

	std::vector<vk::CommandBuffer> commandBuffer;
	commandBuffer = device.allocateCommandBuffers(allocInfo);

	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer[0].begin(beginInfo);

	vk::BufferCopy copyRegion = vk::BufferCopy();
	copyRegion.setSize(size);
	commandBuffer[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	commandBuffer[0].end();

	vk::SubmitInfo submitInfo = vk::SubmitInfo();
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&commandBuffer[0]);

	graphicsQueue.submit(1, &submitInfo, nullptr);
	graphicsQueue.waitIdle();

	device.freeCommandBuffers(commandPool, 1, &commandBuffer[0]);
}

void HVKContext::createInstance()
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

void HVKContext::createSurface()
{
	VkSurfaceKHR sf;
	if (glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &sf) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::SurfaceKHR(sf);
}

void HVKContext::createLogicalDevice()
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

void HVKContext::pickPhysicalDevice()
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

void HVKContext::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

	window = glfwCreateWindow(960, 540, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	keyinput_init(window, 60);
}

void HVKContext::init()
{
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	validationLayers.push_back("VK_LAYER_KHRONOS_validation");
	initWindow();
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();

	createCommandPool();
	createCommandBuffers();

	createSyncObjects();
}

void HVKContext::deinit()
{
	cleanupSwapChain();
	device.destroyRenderPass(renderPass);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		device.destroySemaphore(renderFinishedSemaphores[i]);
		device.destroySemaphore(imageAvailableSemaphores[i]);
		device.destroyFence(inFlightFences[i]);
	}

	device.destroyCommandPool(commandPool, nullptr);

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

bool HVKContext::isClosed()
{
	return glfwWindowShouldClose(window);
}

void HVKContext::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR();
	createInfo.setSurface(surface)
		.setMinImageCount(imageCount)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	QueueFamilyIndices indices = findQueueFamilies();
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else
	{
		createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	}

	createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(vk::True);

	swapChain = device.createSwapchainKHR(createInfo);
	if (!swapChain)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	device.getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	device.getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HVKContext::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
	}
}

void HVKContext::createDepthResources()
{
	vk::Format depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

void HVKContext::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<vk::ImageView, 2> attachments = {swapChainImageViews[i], depthImageView};

		vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo();
		framebufferInfo.setRenderPass(renderPass)
			.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(swapChainExtent.width)
			.setHeight(swapChainExtent.height)
			.setLayers(1);

		swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo, nullptr);
		if (!swapChainFramebuffers[i])
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void HVKContext::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createDepthResources();
	createFramebuffers();
}

void HVKContext::cleanupSwapChain()
{
	device.destroyImageView(depthImageView);
	device.destroyImage(depthImage);
	device.freeMemory(depthImageMemory);

	for (auto framebuffer : swapChainFramebuffers)
	{
		device.destroyFramebuffer(framebuffer);
	}

	for (auto imageView : swapChainImageViews)
	{
		device.destroyImageView(imageView);
	}

	device.destroySwapchainKHR(swapChain);
}

void HVKContext::createRenderPass()
{
	vk::AttachmentDescription colorAttachment = vk::AttachmentDescription();
	colorAttachment.setFormat(swapChainImageFormat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depthAttachment = vk::AttachmentDescription();
	depthAttachment.setFormat(findDepthFormat())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference();
	colorAttachmentRef.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference();
	depthAttachmentRef.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = vk::SubpassDescription();
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency = vk::SubpassDependency();
	dependency.setSrcSubpass(vk::SubpassExternal)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
						 vk::PipelineStageFlagBits::eLateFragmentTests)
		.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
						 vk::PipelineStageFlagBits::eEarlyFragmentTests)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
						  vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo();
	renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	renderPass = device.createRenderPass(renderPassInfo);
	if (!renderPass)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void HVKContext::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();

	vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
	fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (device.createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
			device.createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
			device.createFence(&fenceInfo, nullptr, &inFlightFences[i]) != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void HVKContext::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies();

	vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo();
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

	commandPool = device.createCommandPool(poolInfo);
	if (!commandPool)
	{
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void HVKContext::createCommandBuffers()
{
	commandBuffers.clear();

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
	allocInfo.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);

	commandBuffers = device.allocateCommandBuffers(allocInfo);
	if (commandBuffers.empty())
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void HVKContext::buildCommandBuffer()
{
	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
	if (commandBuffers[currentFrame].begin(&beginInfo) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo();
	renderPassInfo.setRenderPass(renderPass)
		.setFramebuffer(swapChainFramebuffers[imageIndex])
		.setRenderArea(vk::Rect2D().setOffset(vk::Offset2D().setX(0).setY(0)).setExtent(swapChainExtent));

	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
	renderPassInfo.setPClearValues(clearValues.data());

	commandBuffers[currentFrame].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	vk::Viewport viewport = vk::Viewport();
	vk::Extent2D curExtent = swapChainExtent;
	viewport.setX(0.0f)
		.setY(0.0f)
		.setWidth(static_cast<float>(curExtent.width))
		.setHeight(static_cast<float>(curExtent.height))
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);
	commandBuffers[currentFrame].setViewport(0, viewport);

	vk::Rect2D scissor = vk::Rect2D();
	scissor.setOffset(vk::Offset2D().setX(0).setY(0))
		.setExtent(curExtent);
	commandBuffers[currentFrame].setScissor(0, scissor);

	return;
}
