#pragma once
#include "../function/platform/FileService.h"
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "../function/render/RenderHeader.h"

#define CALCULATE_VERTEX_NORMAL

namespace VlkEngine {
	enum ModelTextureType {
		Color=0,
		Cavity=1,
		Goss=2,
		Specular=3,
		MicroNormal=4,
		Normal=5,
		Translucency=6
	};

	class ModelManager {
		friend class VulkanEngine;
		friend class VulkanBase;
		friend class VulkanShadowMap;
	protected:
		std::string modelPath;
		std::string colorTexturePath;

	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<glm::mat4> instanceModelMatrix;


	public:
		void SetModel(std::string modelpath, std::string colortexturepath);
		virtual stbi_uc* LoadModelTexture(ModelTextureType type,int* texWidth, int* texHeight, int* texChannels);
		virtual void SetPbrTexture(std::string cavitytexturePath,
			std::string gosstexturePath,
			std::string speculartexturePath,
			std::string micronormaltexturePath,
			std::string normaltexturePath,
			std::string translucencytexturePath);

	protected:
		void LoadModel();

		float CalcTriangleArea(glm::vec3 veca, glm::vec3 vecb, glm::vec3 vecc);
		glm::vec3 CalcTriangleNormal(glm::vec3 veca, glm::vec3 vecb, glm::vec3 vecc);
	};

}