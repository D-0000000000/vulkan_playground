#ifndef _HVKCOMMON_HPP_
#define _HVKCOMMON_HPP_

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;

	static vk::VertexInputBindingDescription getBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription();
		bindingDescription.setBinding(0)
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex);

		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0] = vk::VertexInputAttributeDescription();
		attributeDescriptions[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, pos));

		attributeDescriptions[1].setBinding(0).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, color));

		return attributeDescriptions;
	}
};

using QueueFamilyIndex = std::optional<uint32_t>;

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

#endif
