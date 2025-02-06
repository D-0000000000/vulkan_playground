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

struct HVKGUISettingsObject
{
	uint32_t pointSize;
};

class HVKGUI
{
public:
	void setContext(std::shared_ptr<HVKContext> dev)
	{
		context = dev;
	}

	void initImGUI();

	void init();

	void deinit();

	void drawFrame();

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

	void createSettingsBuffers();

	vk::DescriptorSetLayout getSettingsDescSetLayout();

	vk::DescriptorSet getSettingsDescSet(uint32_t currentImage);

private:
	std::shared_ptr<HVKContext> context;
	vk::PipelineCache pipelineCache;
	vk::DescriptorPool guiDescriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::RenderPass renderPass;

	std::vector<vk::Buffer> SettingsBuffers;
	std::vector<vk::DeviceMemory> SettingsBuffersMemory;
	std::vector<void *> SettingsBuffersMapped;

	vk::DescriptorSetLayout settingsObjDescSetLayout;
	vk::DescriptorPool settingsObjDescPool;
	std::vector<vk::DescriptorSet> settingsObjDescSets;

	HVKGUISettingsObject settingsObject;

	void createDescriptorPool();

	void createSettingsDescriptorPool();

	void createSettingsDescriptorSetLayout();

	void createSettingsDescriptorSets();

	void updateSettingsBuffers(uint32_t currentImage);
};

#endif
