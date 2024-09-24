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

// void HVKGUI::createDescriptorSetLayout()
// {
// 	vk::DescriptorSetLayoutBinding uboLayoutBinding = vk::DescriptorSetLayoutBinding();
// 	uboLayoutBinding.setBinding(0)
// 		.setDescriptorCount(1)
// 		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
// 		.setPImmutableSamplers(nullptr)
// 		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

// 	vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
// 	layoutInfo.setBindingCount(1)
// 		.setPBindings(&uboLayoutBinding);

// 	descriptorSetLayout = context->getDevice().createDescriptorSetLayout(layoutInfo);
// 	if (!descriptorSetLayout)
// 	{
// 		throw std::runtime_error("failed to create descriptor set layout!");
// 	}
// }

// void HVKGUI::createDescriptorSets()
// {
// 	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
// 	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
// 	allocInfo.setDescriptorPool(descriptorPool)
// 		.setDescriptorSetCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
// 		.setPSetLayouts(layouts.data());

// 	descriptorSets.clear();
// 	descriptorSets = context->getDevice().allocateDescriptorSets(allocInfo);
// 	if (descriptorSets.empty())
// 	{
// 		throw std::runtime_error("failed to allocate descriptor sets!");
// 	}

// 	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
// 	{
// 		vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo();
// 		bufferInfo.setBuffer(uniformBuffers[i])
// 			.setOffset(0)
// 			.setRange(sizeof(UniformBufferObject));

// 		vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet();
// 		descriptorWrite.setDstSet(descriptorSets[i])
// 			.setDstBinding(0)
// 			.setDstArrayElement(0)
// 			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
// 			.setDescriptorCount(1)
// 			.setPBufferInfo(&bufferInfo);

// 		context->getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
// 	}
// }

// void HVKGUI::createGraphicsPipeline()
// {
// 	auto vertShaderCode = readFile("shaders/imgui_vert.spv");
// 	auto fragShaderCode = readFile("shaders/imgui_frag.spv");

// 	vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
// 	vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

// 	vk::PipelineShaderStageCreateInfo vertShaderStageInfo = vk::PipelineShaderStageCreateInfo();
// 	vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex)
// 		.setModule(vertShaderModule)
// 		.setPName("main");

// 	vk::PipelineShaderStageCreateInfo fragShaderStageInfo = vk::PipelineShaderStageCreateInfo();
// 	fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment)
// 		.setModule(fragShaderModule)
// 		.setPName("main");

// 	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

// 	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo();

// 	vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription();
// 	bindingDescription.setBinding(0)
// 		.setStride(sizeof(ImDrawVert))
// 		.setInputRate(vk::VertexInputRate::eVertex);

// 	std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};
// 	attributeDescriptions[0] = vk::VertexInputAttributeDescription();
// 	attributeDescriptions[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(ImDrawVert, pos));
// 	attributeDescriptions[1].setBinding(1).setLocation(0).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(ImDrawVert, uv));
// 	attributeDescriptions[2].setBinding(2).setLocation(0).setFormat(vk::Format::eR8G8B8A8Unorm).setOffset(offsetof(ImDrawVert, col));

// 	vertexInputInfo.setVertexBindingDescriptionCount(1)
// 		.setPVertexBindingDescriptions(&bindingDescription)
// 		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
// 		.setPVertexAttributeDescriptions(attributeDescriptions.data());

// 	vk::PipelineInputAssemblyStateCreateInfo inputAssembly = vk::PipelineInputAssemblyStateCreateInfo();

// 	inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
// 		.setPrimitiveRestartEnable(vk::False);

// 	vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo();
// 	viewportState.setViewportCount(1)
// 		.setScissorCount(1);

// 	vk::PipelineRasterizationStateCreateInfo rasterizer = vk::PipelineRasterizationStateCreateInfo();
// 	rasterizer.setDepthClampEnable(vk::False)
// 		.setRasterizerDiscardEnable(vk::False)
// 		.setPolygonMode(vk::PolygonMode::eFill)
// 		.setLineWidth(1.0f)
// 		.setCullMode(vk::CullModeFlagBits::eBack)
// 		.setFrontFace(vk::FrontFace::eCounterClockwise)
// 		.setDepthBiasEnable(vk::False);

// 	vk::PipelineMultisampleStateCreateInfo multisampling = vk::PipelineMultisampleStateCreateInfo();
// 	multisampling.setSampleShadingEnable(vk::False)
// 		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

// 	vk::PipelineDepthStencilStateCreateInfo depthStencil = vk::PipelineDepthStencilStateCreateInfo();
// 	depthStencil.setDepthTestEnable(vk::True)
// 		.setDepthWriteEnable(vk::True)
// 		.setDepthCompareOp(vk::CompareOp::eLess)
// 		.setDepthBoundsTestEnable(vk::False)
// 		.setStencilTestEnable(vk::False);

// 	vk::PipelineColorBlendAttachmentState colorBlendAttachment = vk::PipelineColorBlendAttachmentState();
// 	colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR |
// 										   vk::ColorComponentFlagBits::eG |
// 										   vk::ColorComponentFlagBits::eB |
// 										   vk::ColorComponentFlagBits::eA)
// 		.setBlendEnable(vk::False);

// 	vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo();
// 	colorBlending.setLogicOpEnable(vk::False)
// 		.setLogicOp(vk::LogicOp::eCopy)
// 		.setAttachmentCount(1)
// 		.setPAttachments(&colorBlendAttachment)
// 		.setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

// 	std::vector<vk::DynamicState> dynamicStates = {
// 		vk::DynamicState::eViewport,
// 		vk::DynamicState::eScissor};
// 	vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo();
// 	dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
// 		.setPDynamicStates(dynamicStates.data());

// 	vk::PushConstantRange range = vk::PushConstantRange();
// 	range.setStageFlags(vk::ShaderStageFlagBits::eVertex)
// 		.setOffset(0)
// 		.setSize(16);

// 	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo();
// 	pipelineLayoutInfo.setSetLayoutCount(1)
// 		.setPSetLayouts(&descriptorSetLayout)
// 		.setPushConstantRangeCount(1)
// 		.setPPushConstantRanges(&range);

// 	pipelineLayout = context->getDevice().createPipelineLayout(pipelineLayoutInfo);
// 	if (!pipelineLayout)
// 	{
// 		throw std::runtime_error("failed to create pipeline layout!");
// 	}

// 	vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo();
// 	pipelineInfo.setStageCount(2)
// 		.setPStages(shaderStages)
// 		.setPVertexInputState(&vertexInputInfo)
// 		.setPInputAssemblyState(&inputAssembly)
// 		.setPViewportState(&viewportState)
// 		.setPRasterizationState(&rasterizer)
// 		.setPMultisampleState(&multisampling)
// 		.setPDepthStencilState(&depthStencil)
// 		.setPColorBlendState(&colorBlending)
// 		.setPDynamicState(&dynamicState)
// 		.setLayout(pipelineLayout)
// 		.setRenderPass(context->getRenderPass())
// 		.setSubpass(0)
// 		.setBasePipelineHandle(nullptr);

// 	graphicsPipeline = context->getDevice().createGraphicsPipeline({}, pipelineInfo).value;
// 	if (!graphicsPipeline)
// 	{
// 		throw std::runtime_error("failed to create graphics pipeline!");
// 	}

// 	context->getDevice().destroyShaderModule(fragShaderModule);
// 	context->getDevice().destroyShaderModule(vertShaderModule);
// }

// void HVKGUI::createVertexBuffer()
// {
// 	std::cout << vertices.size() << "=n\n";
// 	vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

// 	vk::Buffer stagingBuffer;
// 	vk::DeviceMemory stagingBufferMemory;
// 	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

// 	void *data;
// 	context->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
// 	memcpy(data, vertices.data(), (size_t)bufferSize);
// 	context->getDevice().unmapMemory(stagingBufferMemory);

// 	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

// 	context->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

// 	context->getDevice().destroyBuffer(stagingBuffer);
// 	context->getDevice().freeMemory(stagingBufferMemory);
// }

// void HVKGUI::createIndexBuffer()
// {
// 	vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

// 	vk::Buffer stagingBuffer;
// 	vk::DeviceMemory stagingBufferMemory;
// 	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

// 	void *data;
// 	context->getDevice().mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
// 	memcpy(data, indices.data(), (size_t)bufferSize);
// 	context->getDevice().unmapMemory(stagingBufferMemory);

// 	createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);

// 	context->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

// 	context->getDevice().destroyBuffer(stagingBuffer);
// 	context->getDevice().freeMemory(stagingBufferMemory);
// }

// void HVKGUI::createUniformBuffers()
// {
// 	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

// 	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
// 	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
// 	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

// 	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
// 	{
// 		createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers[i], uniformBuffersMemory[i]);

// 		context->getDevice().mapMemory(uniformBuffersMemory[i], 0, bufferSize, {}, &uniformBuffersMapped[i]);
// 	}
// }

// void HVKGUI::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
// {
// 	vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
// 	bufferInfo.setSize(size)
// 		.setUsage(usage)
// 		.setSharingMode(vk::SharingMode::eExclusive);

// 	buffer = context->getDevice().createBuffer(bufferInfo, nullptr);
// 	if (!buffer)
// 	{
// 		throw std::runtime_error("failed to create buffer!");
// 	}

// 	vk::MemoryRequirements memRequirements = context->getDevice().getBufferMemoryRequirements(buffer);

// 	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo();
// 	allocInfo.setAllocationSize(memRequirements.size)
// 		.setMemoryTypeIndex(context->findMemoryType(memRequirements.memoryTypeBits, properties));

// 	bufferMemory = context->getDevice().allocateMemory(allocInfo, nullptr);
// 	if (!bufferMemory)
// 	{
// 		throw std::runtime_error("failed to allocate buffer memory!");
// 	}

// 	context->getDevice().bindBufferMemory(buffer, bufferMemory, 0);
// 	return;
// }

// void HVKGUI::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
// {
// 	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
// 	vk::Buffer vertexBuffers[] = {vertexBuffer};
// 	vk::DeviceSize offsets[] = {0};
// 	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
// 	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
// 	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[context->getCurrentFrame()], 0, nullptr);
// 	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
// 	return;
// }

// void HVKGUI::updateUniformBuffer(uint32_t currentImage)
// {
// 	UniformBufferObject ubo{};
// 	ubo.model = glm::mat4(1.0f);
// 	glm::vec3 eye(0.0f, 0.0f, 0.0f);
// 	glm::vec3 center(0.0f, 0.0f, 0.0f);
// 	glm::vec3 up(0.0f, 0.0f, 0.0f);
// 	get_cam_params(eye, center, up);
// 	ubo.view = glm::lookAt(eye, center, up);
// 	vk::Extent2D curExtent = context->getSwapChainExtent();
// 	ubo.proj = glm::perspective(glm::radians(55.0f), curExtent.width / (float)curExtent.height, 0.1f, 1000.0f);
// 	ubo.proj[1][1] *= -1;
// 	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
// }

// bool HVKGUI::updateBuffers()
// {
// 	// Rendering
// 	ImGui::Render();
// 	ImDrawData *drawData = ImGui::GetDrawData();
// 	if (!drawData)
// 	{
// 		return false;
// 	}
// 	uint32_t vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
// 	uint32_t indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
// 	if ((vertexBufferSize == 0) || (indexBufferSize == 0))
// 	{
// 		return false;
// 	}

// 	if (prevVertexBufferSize != vertexBufferSize)
// 	{
// 		prevVertexBufferSize = vertexBufferSize;
// 		context->getDevice().destroyBuffer(vertexBuffer);
// 		vk::Buffer stagingBuffer;
// 		vk::DeviceMemory stagingBufferMemory;
// 		createBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

// 		void *data;
// 		context->getDevice().mapMemory(stagingBufferMemory, 0, vertexBufferSize, {}, &data);
// 		memcpy(data, vertices.data(), (size_t)vertexBufferSize);
// 		context->getDevice().unmapMemory(stagingBufferMemory);

// 		context->copyBuffer(stagingBuffer, vertexBuffer, vertexBufferSize);
// 	}

// 	if (prevIndexBufferSize != indexBufferSize)
// 	{
// 		prevIndexBufferSize = indexBufferSize;
// 		context->getDevice().destroyBuffer(indexBuffer);
// 		vk::Buffer stagingBuffer;
// 		vk::DeviceMemory stagingBufferMemory;
// 		createBuffer(indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

// 		void *data;
// 		context->getDevice().mapMemory(stagingBufferMemory, 0, indexBufferSize, {}, &data);
// 		memcpy(data, vertices.data(), (size_t)indexBufferSize);
// 		context->getDevice().unmapMemory(stagingBufferMemory);

// 		context->copyBuffer(stagingBuffer, vertexBuffer, indexBufferSize);
// 	}
// }

void HVKGUI::initImGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	// io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;

	ImGui_ImplGlfw_InitForVulkan(context->getWindow(), false);

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
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = VK_NULL_HANDLE;
	initInfo.CheckVkResultFn = VK_NULL_HANDLE;
	ImGui_ImplVulkan_Init(&initInfo);
	return;
}

void HVKGUI::drawFrame()
{
	// uint32_t imageIndex = context->getImageIndex();

	// // updateUniformBuffer(context->getCurrentFrame());
	// updateBuffers();

	// recordCommandBuffer(context->getCurrentCommandBuffer(), imageIndex);
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Hello, world!");			  // Create a window called "Hello, world!" and append into it.
	ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
	ImGui::NewLine();
	ImGui::Text("FUCK");
	// ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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
