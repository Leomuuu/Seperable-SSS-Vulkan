#pragma once

#include "ModelManager.h"

namespace VlkEngine {

	class PbrModelManager :public ModelManager {
	protected:
		std::string cavityTexturePath;
		std::string gossTexturePath;
		std::string specularTexturePath;
		std::string micronormalTexturePath;
		std::string normalTexturePath;
		std::string translucencyTexturePath;

	public:
		void SetPbrTexture(std::string cavitytexturePath,
		std::string gosstexturePath,
		std::string speculartexturePath,
		std::string micronormaltexturePath,
		std::string normaltexturePath,
		std::string translucencytexturePath);

		virtual stbi_uc* LoadModelTexture(ModelTextureType type, int* texWidth, int* texHeight, int* texChannels);

	};
}