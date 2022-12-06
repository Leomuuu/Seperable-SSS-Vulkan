#include "PbrModelManager.h"

namespace VlkEngine {

	void PbrModelManager::SetPbrTexture(std::string cavitytexturePath, std::string gosstexturePath,
		std::string speculartexturePath, std::string micronormaltexturePath, 
		std::string normaltexturePath, std::string translucencytexturePath)
	{
		cavityTexturePath=cavitytexturePath;
		gossTexturePath=gosstexturePath;
		specularTexturePath=speculartexturePath;
		micronormalTexturePath=micronormaltexturePath;
		normalTexturePath=normaltexturePath;
		translucencyTexturePath=translucencytexturePath;
	}

	stbi_uc* PbrModelManager::LoadModelTexture(ModelTextureType type, int* texWidth, int* texHeight, int* texChannels)
	{
		switch (type){
		case Color:
			return FileService::LoadAssetImage(colorTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
		case Cavity:
			return FileService::LoadAssetImage(cavityTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
		case Goss:
			return FileService::LoadAssetImage(gossTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
		case Specular:
			return FileService::LoadAssetImage(specularTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
		case MicroNormal:
			return FileService::LoadAssetImage(micronormalTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
		case Normal:
			return FileService::LoadAssetImage(normalTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
		case Translucency:
			return FileService::LoadAssetImage(translucencyTexturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);


		};
	}
}