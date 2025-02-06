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

void HVKApp::init()
{
	// topology = vk::PrimitiveTopology::ePointList;
	initVulkan();
}

void HVKApp::initVulkan()
{
	createGraphicsPipeline();
	createVertexBuffer();
	createIndexBuffer();
}

void HVKApp::deinit()
{

	context->getDevice().destroyPipeline(graphicsPipeline);
	context->getDevice().destroyPipelineLayout(pipelineLayout);

	context->getDevice().destroyBuffer(indexBuffer);
	context->getDevice().freeMemory(indexBufferMemory);

	context->getDevice().destroyBuffer(vertexBuffer);
	context->getDevice().freeMemory(vertexBufferMemory);
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

	inputAssembly.setTopology(topology)
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
		.setRasterizationSamples(context->getSampleCount());

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
	std::vector<vk::DescriptorSetLayout> descSetLayouts{camera->getCameraDescSetLayout(), gui->getSettingsDescSetLayout()};
	pipelineLayoutInfo.setSetLayoutCount(static_cast<uint32_t>(descSetLayouts.size()))
		.setPSetLayouts(descSetLayouts.data());

	pipelineLayout = context->getDevice().createPipelineLayout(pipelineLayoutInfo);
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
		.setRenderPass(context->getRenderPass())
		.setSubpass(0)
		.setBasePipelineHandle(nullptr);

	graphicsPipeline = context->getDevice().createGraphicsPipeline({}, pipelineInfo).value;
	if (!graphicsPipeline)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	context->getDevice().destroyShaderModule(fragShaderModule);
	context->getDevice().destroyShaderModule(vertShaderModule);
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
	context->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	context->getDevice().unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

	context->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	context->getDevice().destroyBuffer(stagingBuffer);
	context->getDevice().freeMemory(stagingBufferMemory);
}

void HVKApp::createIndexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

	void *data;
	context->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	context->getDevice().unmapMemory(stagingBufferMemory);

	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);

	context->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	context->getDevice().destroyBuffer(stagingBuffer);
	context->getDevice().freeMemory(stagingBufferMemory);
}

void HVKApp::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
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

void HVKApp::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
	vk::Buffer vertexBuffers[] = {vertexBuffer};
	vk::DeviceSize offsets[] = {0};
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
	uint32_t currentFrame = context->getCurrentFrame();
	std::vector<vk::DescriptorSet> descSets{camera->getCameraDescSet(currentFrame), gui->getSettingsDescSet(currentFrame)};
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);
	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	return;
}

void HVKApp::setPrimitiveTopology(vk::PrimitiveTopology top)
{
	topology = top;
	return;
}

void HVKApp::drawFrame()
{
	uint32_t imageIndex = context->getImageIndex();

	recordCommandBuffer(context->getCurrentCommandBuffer(), imageIndex);
}

vk::ShaderModule HVKApp::createShaderModule(const std::vector<char> &code)
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
