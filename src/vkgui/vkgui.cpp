#include "vkgui.hpp"
#include <set>

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

	descriptorPool = context->getDevice().createDescriptorPool(poolInfo);
	if (!descriptorPool)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
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
	initInfo.DescriptorPool = descriptorPool;
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
	ImGui::Begin("Hello, world!");			  // Create a window called "Hello, world!" and append into it.
	ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
	ImGui::NewLine();
	ImGui::Text("Seems good.");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	ImGui::Render();
	ImDrawData *drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, context->getCurrentCommandBuffer());
}

vk::ShaderModule HVKGUI::createShaderModule(const std::vector<char> &code)
{
	vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo();
	createInfo.setCodeSize(code.size())
		.setPCode(reinterpret_cast<const uint32_t *>(code.data()));

	vk::ShaderModule shaderModule;
	if (context->getDevice().createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
