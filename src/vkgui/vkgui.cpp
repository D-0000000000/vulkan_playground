#include "vkgui.hpp"
#include <set>

void HVKGUI::init()
{
	memset(&settingsObject, 0, sizeof(settingsObject));
	settingsObject.pointSize = 1;
	createSettingsBuffers();
	createDescriptorPool();
	createSettingsDescriptorPool();
	createSettingsDescriptorSetLayout();
	createSettingsDescriptorSets();
	initImGUI();
	// initVulkan();
}

void HVKGUI::deinit()
{
	ImGui_ImplVulkan_Shutdown();
	context->getDevice().destroyDescriptorPool(guiDescriptorPool);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		context->getDevice().destroyBuffer(SettingsBuffers[i]);
		context->getDevice().freeMemory(SettingsBuffersMemory[i]);
	}

	context->getDevice().destroyDescriptorPool(settingsObjDescPool);
	context->getDevice().destroyDescriptorSetLayout(settingsObjDescSetLayout);
	return;
}

void HVKGUI::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
{
	vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
	bufferInfo.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);

	buffer = context->getDevice().createBuffer(bufferInfo, nullptr);
	if (!buffer)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	vk::MemoryRequirements memRequirements = context->getDevice().getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo();
	allocInfo.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(context->findMemoryType(memRequirements.memoryTypeBits, properties));

	bufferMemory = context->getDevice().allocateMemory(allocInfo, nullptr);
	if (!bufferMemory)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	context->getDevice().bindBufferMemory(buffer, bufferMemory, 0);
	return;
}

void HVKGUI::createSettingsBuffers()
{
	vk::DeviceSize settingsObjSize = sizeof(HVKGUISettingsObject);
	SettingsBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	SettingsBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	SettingsBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(settingsObjSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, SettingsBuffers[i], SettingsBuffersMemory[i]);
		context->getDevice().mapMemory(SettingsBuffersMemory[i], 0, settingsObjSize, vk::MemoryMapFlags(0), &SettingsBuffersMapped[i]);
	}
	return;
}

void HVKGUI::createSettingsDescriptorPool()
{
	vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize();
	poolSize.setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
	poolInfo.setPoolSizeCount(1)
		.setPPoolSizes(&poolSize)
		.setMaxSets(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	settingsObjDescPool = context->getDevice().createDescriptorPool(poolInfo);
	if (!settingsObjDescPool)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void HVKGUI::createDescriptorPool()
{
	vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize();
	// poolSize.setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
	poolSize.setDescriptorCount(static_cast<uint32_t>(1))
		.setType(vk::DescriptorType::eUniformBuffer);

	vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
	poolInfo.setPPoolSizes(&poolSize)
		.setMaxSets(static_cast<uint32_t>(2))
		.setPoolSizeCount(1)
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	guiDescriptorPool = context->getDevice().createDescriptorPool(poolInfo);
	if (!guiDescriptorPool)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void HVKGUI::createSettingsDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding();
	uboLayoutBinding.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
	layoutInfo.setBindingCount(1)
		.setPBindings(&uboLayoutBinding);

	settingsObjDescSetLayout = context->getDevice().createDescriptorSetLayout(layoutInfo);
	if (!settingsObjDescSetLayout)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void HVKGUI::createSettingsDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, settingsObjDescSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
	allocInfo.setDescriptorPool(settingsObjDescPool)
		.setDescriptorSetCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
		.setPSetLayouts(layouts.data());

	settingsObjDescSets.clear();
	settingsObjDescSets = context->getDevice().allocateDescriptorSets(allocInfo);
	if (settingsObjDescSets.empty())
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo();
		bufferInfo.setBuffer(SettingsBuffers[i])
			.setOffset(0)
			.setRange(sizeof(HVKGUISettingsObject));

		vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet();
		descriptorWrite.setDstSet(settingsObjDescSets[i])
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPBufferInfo(&bufferInfo);

		context->getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	}
}

void HVKGUI::updateSettingsBuffers(uint32_t currentImage)
{
	memcpy(SettingsBuffersMapped[currentImage], &settingsObject, sizeof(settingsObject));
	return;
}

vk::DescriptorSetLayout HVKGUI::getSettingsDescSetLayout()
{
	return settingsObjDescSetLayout;
}

vk::DescriptorSet HVKGUI::getSettingsDescSet(uint32_t currentImage)
{
	return settingsObjDescSets[currentImage];
}

void HVKGUI::initImGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	ImGui_ImplGlfw_InitForVulkan(context->getWindow(), true);

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = context->getInstance();
	initInfo.PhysicalDevice = context->getPhysicalDevice();
	initInfo.Device = context->getDevice();
	initInfo.QueueFamily = context->getGraphicsQueueFamily();
	initInfo.Queue = context->getGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = guiDescriptorPool;
	initInfo.RenderPass = context->getRenderPass();
	initInfo.Subpass = 0;
	auto swapChainSupport = context->querySwapChainSupport();
	initInfo.MinImageCount = swapChainSupport.capabilities.minImageCount;
	initInfo.ImageCount = 2;
	initInfo.MSAASamples = (VkSampleCountFlagBits)context->getSampleCount();
	initInfo.Allocator = VK_NULL_HANDLE;
	initInfo.CheckVkResultFn = VK_NULL_HANDLE;
	ImGui_ImplVulkan_Init(&initInfo);
	return;
}

void HVKGUI::drawFrame()
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::NewLine();
	int pointSize = settingsObject.pointSize;
	ImGui::SliderInt("PointSize", &pointSize, 1, 10, "%d", ImGuiSliderFlags_AlwaysClamp);
	settingsObject.pointSize = pointSize;
	updateSettingsBuffers(context->getCurrentFrame());

	ImGui::End();

	ImGui::Render();
	ImDrawData *drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, context->getCurrentCommandBuffer());
}
