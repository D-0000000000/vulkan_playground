#include "vkrun.hpp"
#include <vulkan/vulkan.hpp>

void HVKApp::setIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind)
{
	vertices.clear();
	indices.clear();
	vertices = vx;
	indices = ind;
	std::cout << vertices.size() << " " << vx.size() << "\n";
	return;
}

void HVKApp::initVulkan()
{
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
}

void HVKApp::cleanup()
{

	phyDev->getDevice().destroyPipeline(graphicsPipeline);
	phyDev->getDevice().destroyPipelineLayout(pipelineLayout);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		phyDev->getDevice().destroyBuffer(uniformBuffers[i]);
		phyDev->getDevice().freeMemory(uniformBuffersMemory[i]);
	}

	phyDev->getDevice().destroyDescriptorPool(descriptorPool);

	phyDev->getDevice().destroyDescriptorSetLayout(descriptorSetLayout);

	phyDev->getDevice().destroyBuffer(indexBuffer);
	phyDev->getDevice().freeMemory(indexBufferMemory);

	phyDev->getDevice().destroyBuffer(vertexBuffer);
	phyDev->getDevice().freeMemory(vertexBufferMemory);
}

void HVKApp::createDescriptorSetLayout()
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

	descriptorSetLayout = phyDev->getDevice().createDescriptorSetLayout(layoutInfo);
	if (!descriptorSetLayout)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void HVKApp::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo = vk::PipelineShaderStageCreateInfo();
	vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex)
		.setModule(vertShaderModule)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo = vk::PipelineShaderStageCreateInfo();
	fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment)
		.setModule(fragShaderModule)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo();

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.setVertexBindingDescriptionCount(1)
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo();

	inputAssembly.setTopology(vk::PrimitiveTopology::ePointList)
		.setPrimitiveRestartEnable(vk::False);

	vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo();
	viewportState.setViewportCount(1)
		.setScissorCount(1);

	vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo();
	rasterizer.setDepthClampEnable(vk::False)
		.setRasterizerDiscardEnable(vk::False)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(vk::False);

	vk::PipelineMultisampleStateCreateInfo multisampling = vk::PipelineMultisampleStateCreateInfo();
	multisampling.setSampleShadingEnable(vk::False)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo();
	depthStencil.setDepthTestEnable(vk::True)
		.setDepthWriteEnable(vk::True)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(vk::False)
		.setStencilTestEnable(vk::False);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment = vk::PipelineColorBlendAttachmentState();
	colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR |
										   vk::ColorComponentFlagBits::eG |
										   vk::ColorComponentFlagBits::eB |
										   vk::ColorComponentFlagBits::eA)
		.setBlendEnable(vk::False);

	vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo();
	colorBlending.setLogicOpEnable(vk::False)
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttachment)
		.setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor};
	vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo();
	dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
		.setPDynamicStates(dynamicStates.data());

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo();
	pipelineLayoutInfo.setSetLayoutCount(1)
		.setPSetLayouts(&descriptorSetLayout);

	pipelineLayout = phyDev->getDevice().createPipelineLayout(pipelineLayoutInfo);
	if (!pipelineLayout)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo();
	pipelineInfo.setStageCount(2)
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicState)
		.setLayout(pipelineLayout)
		.setRenderPass(phyDev->getRenderPass())
		.setSubpass(0)
		.setBasePipelineHandle(nullptr);

	graphicsPipeline = phyDev->getDevice().createGraphicsPipeline({}, pipelineInfo).value;
	if (!graphicsPipeline)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	phyDev->getDevice().destroyShaderModule(fragShaderModule);
	phyDev->getDevice().destroyShaderModule(vertShaderModule);
}

bool HVKApp::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32Sfloat || format == vk::Format::eD24UnormS8Uint;
}

void HVKApp::createVertexBuffer()
{
	std::cout << vertices.size() << "=n\n";
	vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

	void *data;
	phyDev->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	phyDev->getDevice().unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

	phyDev->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	phyDev->getDevice().destroyBuffer(stagingBuffer);
	phyDev->getDevice().freeMemory(stagingBufferMemory);
}

void HVKApp::createIndexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

	void *data;
	phyDev->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	phyDev->getDevice().unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);

	phyDev->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	phyDev->getDevice().destroyBuffer(stagingBuffer);
	phyDev->getDevice().freeMemory(stagingBufferMemory);
}

void HVKApp::createUniformBuffers()
{
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i], uniformBuffersMemory[i]);

		phyDev->getDevice().mapMemory(uniformBuffersMemory[i], 0, bufferSize, {}, &uniformBuffersMapped[i]);
	}
}

void HVKApp::createDescriptorPool()
{
	vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize();
	poolSize.setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
	poolInfo.setPoolSizeCount(1)
		.setPPoolSizes(&poolSize)
		.setMaxSets(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	descriptorPool = phyDev->getDevice().createDescriptorPool(poolInfo);
	if (!descriptorPool)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void HVKApp::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
	allocInfo.setDescriptorPool(descriptorPool)
		.setDescriptorSetCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
		.setPSetLayouts(layouts.data());

	descriptorSets.clear();
	descriptorSets = phyDev->getDevice().allocateDescriptorSets(allocInfo);
	if (descriptorSets.empty())
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo();
		bufferInfo.setBuffer(uniformBuffers[i])
			.setOffset(0)
			.setRange(sizeof(UniformBufferObject));

		vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet();
		descriptorWrite.setDstSet(descriptorSets[i])
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPBufferInfo(&bufferInfo);

		phyDev->getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	}
}

void HVKApp::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
{
	vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
	bufferInfo.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);

	buffer = phyDev->getDevice().createBuffer(bufferInfo, nullptr);
	if (!buffer)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	vk::MemoryRequirements memRequirements = phyDev->getDevice().getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo();
	allocInfo.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(phyDev->findMemoryType(memRequirements.memoryTypeBits, properties));

	bufferMemory = phyDev->getDevice().allocateMemory(allocInfo, nullptr);
	if (!bufferMemory)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	phyDev->getDevice().bindBufferMemory(buffer, bufferMemory, 0);
	return;
}

void HVKApp::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
	vk::Buffer vertexBuffers[] = {vertexBuffer};
	vk::DeviceSize offsets[] = {0};
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[phyDev->getCurrentFrame()], 0, nullptr);
	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	return;
}

void HVKApp::updateUniformBuffer(uint32_t currentImage)
{
	UniformBufferObject ubo{};
	ubo.model = glm::mat4(1.0f);
	glm::vec3 eye(0.0f, 0.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 0.0f, 0.0f);
	get_cam_params(eye, center, up);
	ubo.view = glm::lookAt(eye, center, up);
	vk::Extent2D curExtent = phyDev->getSwapChainExtent();
	ubo.proj = glm::perspective(glm::radians(55.0f), curExtent.width / (float)curExtent.height, 0.1f, 1000.0f);
	ubo.proj[1][1] *= -1;
	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void HVKApp::drawFrame()
{
	uint32_t imageIndex = phyDev->getImageIndex();

	updateUniformBuffer(phyDev->getCurrentFrame());

	recordCommandBuffer(phyDev->getCurrentCommandBuffer(), imageIndex);
}

vk::ShaderModule HVKApp::createShaderModule(const std::vector<char> &code)
{
	vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo();
	createInfo.setCodeSize(code.size())
		.setPCode(reinterpret_cast<const uint32_t *>(code.data()));

	vk::ShaderModule shaderModule;
	if (phyDev->getDevice().createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
