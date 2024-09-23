#include "vkgui.hpp"
#include <set>

// void VKGUI::initVulkan()
// {
// 	vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo();
// 	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient)
// 		.setQueueFamilyIndex(phyDev->getGraphicsQueueFamily());
// 	commandPool = phyDev->getDevice().createCommandPool(poolInfo);
// }

// void VKGUI::initImGUI()
// {
// 	ImGui::CreateContext();
// 	ImGuiIO &io = ImGui::GetIO();
// 	(void)io;
// 	ImGui_ImplGlfw_InitForVulkan(phyDev->getWindow(), false);
// 	ImGui_ImplVulkan_InitInfo initInfo = {};
// 	// ImGui_ImplVulkan_Init()
// 	return;
// }
