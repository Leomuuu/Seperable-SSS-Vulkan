#pragma once

#include "Component.h"
#include "../../render/RenderHeader.h"
#include "../../platform/FileService.h"
#include "../../../VulkanEngine.h"

#define CALCULATE_VERTEX_NORMAL

namespace VlkEngine {
	class RenderComponent:public Component {
	protected:
		std::string modelPath;

		std::string colorTexturePath;
		std::string cavityTexturePath;
		std::string gossTexturePath;
		std::string specularTexturePath;
		std::string micronormalTexturePath;
		std::string normalTexturePath;
		std::string translucencyTexturePath;

	public:
		void SetModel(std::string modelpath);
		void SetTexture(
			std::string colortexturePath,
			std::string cavitytexturePath,
			std::string gosstexturePath,
			std::string speculartexturePath,
			std::string micronormaltexturePath,
			std::string normaltexturePath,
			std::string translucencytexturePath);




	};
}