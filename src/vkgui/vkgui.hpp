#ifndef _VKGUI_HPP_
#define _VKGUI_HPP_

#include "vkdev/vkdev.hpp"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/imgui.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <memory>

class VKGUI
{
public:
	void setPhyDev(std::shared_ptr<HVKPhyDev> dev)
	{
		phyDev = dev;
	}

	void initVulkan()
	{
		// vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo();
		// poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient)
		// 	.setQueueFamilyIndex(phyDev->getGraphicsQueueFamily());
		// commandPool = phyDev->getDevice().createCommandPool(poolInfo);
		// vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
		// allocInfo.setCommandBufferCount(1)
		// 	.setCommandPool(commandPool)
		// 	.setLevel(vk::CommandBufferLevel::ePrimary);
		// phyDev->getDevice().allocateCommandBuffers(&allocInfo, &commandBuffer);
		// vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
		// fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
		// fence = phyDev->getDevice().createFence(fenceInfo);

		// vk::RenderPassCreateInfo passInfo = vk::RenderPassCreateInfo();
		// passInfo.set
	}

	void initImGUI()
	{
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void)io;
		ImGui_ImplGlfw_InitForVulkan(phyDev->getWindow(), false);
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = phyDev->getInstance();
		initInfo.PhysicalDevice = phyDev->getPhysicalDevice();
		initInfo.Device = phyDev->getDevice();
		initInfo.QueueFamily = phyDev->getGraphicsQueueFamily();
		initInfo.Queue = phyDev->getGraphicsQueue();
		initInfo.PipelineCache = pipelineCache;
		initInfo.DescriptorPool = descPool;
		initInfo.RenderPass = renderPass;
		initInfo.Subpass = 0;
		SwapChainSupportDetails swapChainSupport = phyDev->querySwapChainSupport();
		initInfo.MinImageCount = swapChainSupport.capabilities.minImageCount;
		initInfo.ImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		// initInfo.Allocator = g_Allocator;
		// initInfo.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&initInfo);
		return;
	}

private:
	std::shared_ptr<HVKPhyDev> phyDev;
	vk::CommandPool commandPool;
	vk::CommandBuffer commandBuffer;
	vk::PipelineCache pipelineCache;
	vk::DescriptorPool descPool;
	vk::RenderPass renderPass;
};

#endif
