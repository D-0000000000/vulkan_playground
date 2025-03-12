#include "vkmesh.hpp"

#define STB_IMAGE_IMPLEMENTATION
#ifdef _MSC_VER
#include <stb_image.h>
#else
#include <stb/stb_image.h>
#endif

#include <spng.h>
#include <vulkan/vulkan.hpp>

void HVKMesh::setIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind)
{
	vertices.clear();
	indices.clear();
	vertices = vx;
	indices = ind;
	std::cout << vertices.size() << " " << vx.size() << "\n";
	return;
}

void HVKMesh::setMeshTexturePath(std::string mPath, std::string tPath)
{
	meshPath = mPath;
	texturePath = tPath;
	return;
}

void HVKMesh::init()
{
	initVulkan();
}

void HVKMesh::initVulkan()
{
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createVertexBuffer();
	createIndexBuffer();
	createDescriptorPool();
	createDescriptorSets();
}

void HVKMesh::deinit()
{

	context->getDevice().destroyPipeline(graphicsPipeline);
	context->getDevice().destroyPipelineLayout(pipelineLayout);

	context->getDevice().destroyDescriptorPool(descriptorPool);

	context->getDevice().destroyDescriptorSetLayout(descriptorSetLayout);

	context->getDevice().destroyBuffer(indexBuffer);
	context->getDevice().freeMemory(indexBufferMemory);

	context->getDevice().destroyBuffer(vertexBuffer);
	context->getDevice().freeMemory(vertexBufferMemory);

	context->getDevice().destroySampler(textureSampler);
	context->getDevice().destroyImageView(textureImageView);
	context->getDevice().destroyImage(textureImage);
	context->getDevice().freeMemory(textureImageMemory);
}

void HVKMesh::createTextureImage()
{
	spng_ctx *ctx = spng_ctx_new(0);
	spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);
	FILE *png = fopen(texturePath.c_str(), "rb");
	spng_set_png_file(ctx, png);
	struct spng_ihdr ihdr;
	int ret = spng_get_ihdr(ctx, &ihdr);
	size_t image_size = 0;
	ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
	std::cout << image_size << " image_size\n";
	vk::DeviceSize imageSize = image_size;
	auto image = new unsigned char[imageSize];
	ret = spng_decode_image(ctx, image, image_size, SPNG_FMT_RGBA8, 0);
	auto revimage = new unsigned char[imageSize];
	memcpy(revimage, image, imageSize);
	for (int i = 0; i < ihdr.height; i++)
	{
		memcpy(image + i * ihdr.width * 4, revimage + (ihdr.height - i - 1) * ihdr.width * 4, ihdr.width * 4);
	}
	delete[] revimage;
	spng_ctx_free(ctx);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(ihdr.width, ihdr.height)))) + 1;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);
	void *data;
	context->getDevice().mapMemory(stagingBufferMemory, 0, imageSize, {}, &data);
	memcpy(data, image, imageSize);
	context->getDevice().unmapMemory(stagingBufferMemory);
	// stbi_image_free(pixels);
	delete[] image;

	context->createImage(ihdr.width, ihdr.height, mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);
	context->transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
	context->copyBufferToImage(stagingBuffer, textureImage, ihdr.width, ihdr.height);
	// context->transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
	context->generateMipmaps(textureImage, vk::Format::eR8G8B8A8Srgb, ihdr.width, ihdr.height, mipLevels);

	context->getDevice().destroyBuffer(stagingBuffer);
	context->getDevice().freeMemory(stagingBufferMemory);

	return;
}

void HVKMesh::createTextureImageView()
{
	textureImageView = context->createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
}

void HVKMesh::createTextureSampler()
{
	vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo();
	vk::PhysicalDeviceProperties properties = context->getPhysicalDevice().getProperties();
	samplerInfo.setMagFilter(vk::Filter::eLinear)
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(vk::True)
		.setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(vk::False)
		.setCompareEnable(vk::False)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);
	textureSampler = context->getDevice().createSampler(samplerInfo);
}

void HVKMesh::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding samplerLayoutBinding = vk::DescriptorSetLayoutBinding();
	samplerLayoutBinding.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};

	vk::DescriptorSetLayoutCreateInfo layoutInfo = vk::DescriptorSetLayoutCreateInfo();
	layoutInfo.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());

	descriptorSetLayout = context->getDevice().createDescriptorSetLayout(layoutInfo);
	if (!descriptorSetLayout)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void HVKMesh::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/texture_vert.spv");
	auto fragShaderCode = readFile("shaders/texture_frag.spv");

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

	inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
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
	std::vector<vk::DescriptorSetLayout> descSetLayouts = {camera->getCameraDescSetLayout(), descriptorSetLayout};
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

bool HVKMesh::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32Sfloat || format == vk::Format::eD24UnormS8Uint;
}

void HVKMesh::createVertexBuffer()
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

void HVKMesh::createIndexBuffer()
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

void HVKMesh::createDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 1> poolSizes = {vk::DescriptorPoolSize()};

	poolSizes[0].setDescriptorCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)).setType(vk::DescriptorType::eCombinedImageSampler);

	vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo();
	poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	descriptorPool = context->getDevice().createDescriptorPool(poolInfo);
	if (!descriptorPool)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void HVKMesh::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo();
	allocInfo.setDescriptorPool(descriptorPool)
		.setDescriptorSetCount(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT))
		.setPSetLayouts(layouts.data());

	descriptorSets.clear();
	descriptorSets = context->getDevice().allocateDescriptorSets(allocInfo);
	if (descriptorSets.empty())
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo();
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(textureImageView)
			.setSampler(textureSampler);

		std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {vk::WriteDescriptorSet()};
		descriptorWrites[0].setDstSet(descriptorSets[i]).setDstBinding(1).setDstArrayElement(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setPImageInfo(&imageInfo);

		context->getDevice().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void HVKMesh::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
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

void HVKMesh::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
	vk::Buffer vertexBuffers[] = {vertexBuffer};
	vk::DeviceSize offsets[] = {0};
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
	uint32_t currentFrame = context->getCurrentFrame();
	std::vector<vk::DescriptorSet> descSets = {camera->getCameraDescSet(currentFrame), descriptorSets[currentFrame]};
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, static_cast<uint32_t>(descSets.size()), descSets.data(), 0, nullptr);
	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	return;
}

void HVKMesh::drawFrame()
{
	uint32_t imageIndex = context->getImageIndex();

	recordCommandBuffer(context->getCurrentCommandBuffer(), imageIndex);
}

vk::ShaderModule HVKMesh::createShaderModule(const std::vector<char> &code)
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
