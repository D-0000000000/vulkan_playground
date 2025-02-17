#include "vkcam.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void HVKCamera::init()
{
	eye = glm::vec3(0.0f, 0.0f, 0.0f);
	center = glm::vec3(1.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 0.0f, 1.0f);
	createCamObjectBuffers();
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSets();
	return;
}

void HVKCamera::deinit()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		context->getDevice().destroyBuffer(camObjectBuffers[i]);
		context->getDevice().freeMemory(camObjectBuffersMemory[i]);
	}

	context->getDevice().destroyDescriptorPool(camObjDescPool);
	context->getDevice().destroyDescriptorSetLayout(camObjDescSetLayout);
	return;
}

void HVKCamera::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
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

void HVKCamera::createCamObjectBuffers()
{
	vk::DeviceSize camObjSize = sizeof(HVKCameraObject);
	camObjectBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	camObjectBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	camObjectBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(camObjSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, camObjectBuffers[i], camObjectBuffersMemory[i]);
		context->getDevice().mapMemory(camObjectBuffersMemory[i], 0, camObjSize, vk::MemoryMapFlags(0), &camObjectBuffersMapped[i]);
	}
	return;
}

void HVKCamera::createDescriptorPool()
{
	vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize();
	poolSize.setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
	poolInfo.setPoolSizeCount(1)
		.setPPoolSizes(&poolSize)
		.setMaxSets(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	camObjDescPool = context->getDevice().createDescriptorPool(poolInfo);
	if (!camObjDescPool)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void HVKCamera::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding();
	uboLayoutBinding.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
	layoutInfo.setBindingCount(1)
		.setPBindings(&uboLayoutBinding);

	camObjDescSetLayout = context->getDevice().createDescriptorSetLayout(layoutInfo);
	if (!camObjDescSetLayout)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void HVKCamera::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, camObjDescSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
	allocInfo.setDescriptorPool(camObjDescPool)
		.setDescriptorSetCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
		.setPSetLayouts(layouts.data());

	camObjDescSets.clear();
	camObjDescSets = context->getDevice().allocateDescriptorSets(allocInfo);
	if (camObjDescSets.empty())
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo();
		bufferInfo.setBuffer(camObjectBuffers[i])
			.setOffset(0)
			.setRange(sizeof(HVKCameraObject));

		vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet();
		descriptorWrite.setDstSet(camObjDescSets[i])
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPBufferInfo(&bufferInfo);

		context->getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	}
}

void HVKCamera::updateCamObjectBuffers(uint32_t currentImage)
{
	HVKCameraObject cameraObject{};
	cameraObject.model = glm::mat4(1.0f);
	cameraObject.view = glm::lookAt(eye, center, up);
	vk::Extent2D curExtent = context->getSwapChainExtent();
	cameraObject.proj = glm::perspective(glm::radians(55.0f), curExtent.width / (float)curExtent.height, 0.01f, 100000.0f);
	cameraObject.proj[1][1] *= -1;
	memcpy(camObjectBuffersMapped[currentImage], &cameraObject, sizeof(cameraObject));
	return;
}

vk::DescriptorSetLayout HVKCamera::getCameraDescSetLayout()
{
	return camObjDescSetLayout;
}

vk::DescriptorSet HVKCamera::getCameraDescSet(uint32_t currentImage)
{
	return camObjDescSets[currentImage];
}

void HVKCamera::yaw(double y)
{
	return;
}

void HVKCamera::roll(double r)
{
	return;
}

void HVKCamera::pitch(double p)
{
	return;
}

void HVKCamera::move(glm::vec3 dir)
{
	return;
}

void HVKCamera::resetCamera()
{
	eye = glm::vec3(0.0f, 0.0f, 0.0f);
	center = glm::vec3(1.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 0.0f, 1.0f);
	return;
}

void HVKCamera::getLookAt(glm::vec3 &e, glm::vec3 &c, glm::vec3 &u)
{
	e = eye;
	c = center;
	u = up;
	return;
}

void HVKCamera::setLookAt(glm::vec3 &e, glm::vec3 &c, glm::vec3 &u)
{
	eye = e;
	center = c;
	up = u;
	return;
}
