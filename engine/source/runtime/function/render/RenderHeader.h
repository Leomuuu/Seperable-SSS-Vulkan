#pragma once
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define NOMINMAX
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <optional>
#include <string>
#include <array>
#include <map>
#include <set>
#include <cstdint>
#include <limits> 
#include <algorithm> 
#include <chrono>

#define MPI 3.14159265358979323

// validation layer
#define  NDEBUG

namespace VlkEngine {
	const int MAX_FRAMES_IN_FLIGHT = 2;

	const std::string SHADERDIR = "../../../engine/shader/";
	const std::string MODELDIR = "../../../engine/asset/";

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 texCoord;

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color &&
				normal==other.normal && texCoord == other.texCoord;
		}

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, normal);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

	struct ShadowMVP {
		glm::mat4 worldprojview;
		glm::mat4 lightprojview;
	};

	struct MVPMatrix {
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct DynamicUBO {
		glm::mat4* model=nullptr;
	};

	struct FragUniform {
		glm::vec3 viewPosition;
		glm::vec3 lightPosition;
		glm::vec3 lightRadiance;
	};

	struct SSSBlurPushConsts
	{
		glm::vec2 dir;
		glm::vec2 windowSize;
		float sssWidth;
	};

	struct SSSStranslucencyPushConsts {
		glm::vec2 windowSize;
		glm::vec3 translucency_sssWidth_lightFarPlane;
		glm::vec3 lightworldpos;
	};

}

namespace std {
	template<> struct hash<VlkEngine::Vertex> {
		size_t operator()(VlkEngine::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}