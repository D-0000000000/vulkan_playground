#include "vkrun.hpp"
#include <vulkan/vulkan.hpp>

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

void setIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind)
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
	frame_rate = 60;
	frame_time = 1000000000.0 / frame_rate;
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

void HVKApp::mainLoop()
{
	while (!glfwWindowShouldClose(phyDev->getWindow()))
	{
		glfwPollEvents();

		drawFrame();
		frame_count++;
	}

	phyDev->getDevice().waitIdle();
}

void HVKApp::cleanupSwapChain()
{
	phyDev->getDevice().destroyImageView(depthImageView);
	phyDev->getDevice().destroyImage(depthImage);
	phyDev->getDevice().freeMemory(depthImageMemory);

	for (auto framebuffer : swapChainFramebuffers)
	{
		phyDev->getDevice().destroyFramebuffer(framebuffer);
	}

	for (auto imageView : swapChainImageViews)
	{
		phyDev->getDevice().destroyImageView(imageView);
	}

	phyDev->getDevice().destroySwapchainKHR(swapChain);
}

void HVKApp::cleanup()
{
	cleanupSwapChain();

	phyDev->getDevice().destroyPipeline(graphicsPipeline);
	phyDev->getDevice().destroyPipelineLayout(pipelineLayout);
	phyDev->getDevice().destroyRenderPass(renderPass);

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

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		phyDev->getDevice().destroySemaphore(renderFinishedSemaphores[i]);
		phyDev->getDevice().destroySemaphore(imageAvailableSemaphores[i]);
		phyDev->getDevice().destroyFence(inFlightFences[i]);
	}

	phyDev->getDevice().destroyCommandPool(commandPool, nullptr);
}

void HVKApp::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(phyDev->getWindow(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(phyDev->getWindow(), &width, &height);
		glfwWaitEvents();
	}

	phyDev->getDevice().waitIdle();

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createDepthResources();
	createFramebuffers();
}

void HVKApp::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = phyDev->querySwapChainSupport();

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR();
	createInfo.setSurface(phyDev->getSurface())
		.setMinImageCount(imageCount)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	QueueFamilyIndices indices = phyDev->findQueueFamilies();
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

	swapChain = phyDev->getDevice().createSwapchainKHR(createInfo);
	if (!swapChain)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	phyDev->getDevice().getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	phyDev->getDevice().getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HVKApp::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
	}
}

void HVKApp::createRenderPass()
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

	renderPass = phyDev->getDevice().createRenderPass(renderPassInfo);
	if (!renderPass)
	{
		throw std::runtime_error("failed to create render pass!");
	}
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
		.setRenderPass(renderPass)
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

void HVKApp::createFramebuffers()
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

		swapChainFramebuffers[i] = phyDev->getDevice().createFramebuffer(framebufferInfo, nullptr);
		if (!swapChainFramebuffers[i])
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void HVKApp::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = phyDev->findQueueFamilies();

	vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo();
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

	commandPool = phyDev->getDevice().createCommandPool(poolInfo);
	if (!commandPool)
	{
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void HVKApp::createDepthResources()
{
	vk::Format depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

vk::Format HVKApp::findSupportedFormat(const std::vector<vk::Format> &&candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties props;
		phyDev->getPhysicalDevice().getFormatProperties(format, &props);

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

vk::Format HVKApp::findDepthFormat()
{
	return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool HVKApp::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32Sfloat || format == vk::Format::eD24UnormS8Uint;
}

vk::ImageView HVKApp::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
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
	vk::ImageView imageView = phyDev->getDevice().createImageView(viewInfo, nullptr);
	if (!imageView)
	{
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}

void HVKApp::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory)
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

	image = phyDev->getDevice().createImage(imageInfo, nullptr);
	if (!image)
	{
		throw std::runtime_error("failed to create image!");
	}

	vk::MemoryRequirements memRequirements = phyDev->getDevice().getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo();
	allocInfo.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
	imageMemory = phyDev->getDevice().allocateMemory(allocInfo, nullptr);
	if (!imageMemory)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	phyDev->getDevice().bindImageMemory(image, imageMemory, 0);
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

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

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

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

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
		.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

	bufferMemory = phyDev->getDevice().allocateMemory(allocInfo, nullptr);
	if (!bufferMemory)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	phyDev->getDevice().bindBufferMemory(buffer, bufferMemory, 0);
	return;
}

void HVKApp::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(commandPool)
		.setCommandBufferCount(1);

	std::vector<vk::CommandBuffer> commandBuffer;
	commandBuffer = phyDev->getDevice().allocateCommandBuffers(allocInfo);

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

	phyDev->getGraphicsQueue().submit(1, &submitInfo, nullptr);
	phyDev->getGraphicsQueue().waitIdle();

	phyDev->getDevice().freeCommandBuffers(commandPool, 1, &commandBuffer[0]);
}

uint32_t HVKApp::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = phyDev->getPhysicalDevice().getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void HVKApp::createCommandBuffers()
{
	commandBuffers.clear();

	vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo();
	allocInfo.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);

	commandBuffers = phyDev->getDevice().allocateCommandBuffers(allocInfo);
	if (commandBuffers.empty())
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void HVKApp::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
	if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess)
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

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

	vk::Viewport viewport = vk::Viewport();
	viewport.setX(0.0f)
		.setY(0.0f)
		.setWidth(static_cast<float>(swapChainExtent.width))
		.setHeight(static_cast<float>(swapChainExtent.height))
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);
	commandBuffer.setViewport(0, viewport);

	vk::Rect2D scissor = vk::Rect2D();
	scissor.setOffset(vk::Offset2D().setX(0).setY(0))
		.setExtent(swapChainExtent);
	commandBuffer.setScissor(0, scissor);

	vk::Buffer vertexBuffers[] = {vertexBuffer};
	vk::DeviceSize offsets[] = {0};
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	commandBuffer.endRenderPass();

	commandBuffer.end();
	// if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	// {
	// 	throw std::runtime_error("failed to record command buffer!");
	// }
}

void HVKApp::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();

	vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
	fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (phyDev->getDevice().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
			phyDev->getDevice().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
			phyDev->getDevice().createFence(&fenceInfo, nullptr, &inFlightFences[i]) != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
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
	ubo.proj = glm::perspective(glm::radians(55.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void HVKApp::drawFrame()
{
	phyDev->getDevice().waitForFences(inFlightFences[currentFrame], vk::True, UINT64_MAX);

	uint32_t imageIndex;
	vk::Result result = phyDev->getDevice().acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	updateUniformBuffer(currentFrame);

	phyDev->getDevice().resetFences(inFlightFences[currentFrame]);

	commandBuffers[currentFrame].reset(static_cast<vk::CommandBufferResetFlagBits>(0));
	recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	vk::SubmitInfo submitInfo = vk::SubmitInfo();

	vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	submitInfo.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(waitSemaphores)
		.setWaitDstStageMask(waitStages)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&commandBuffers[currentFrame]);

	vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(signalSemaphores);

	if (phyDev->getGraphicsQueue().submit(1, &submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR();
	presentInfo.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(signalSemaphores);

	vk::SwapchainKHR swapChains[] = {swapChain};
	presentInfo.setSwapchainCount(1)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex);

	result = phyDev->getPresentQueue().presentKHR(presentInfo);

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || phyDev->checkFrameBufferResized())
	{
		phyDev->setFrameBufferResized(false);
		recreateSwapChain();
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

vk::SurfaceFormatKHR HVKApp::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
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

vk::PresentModeKHR HVKApp::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
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

vk::Extent2D HVKApp::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(phyDev->getWindow(), &width, &height);

		vk::Extent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
