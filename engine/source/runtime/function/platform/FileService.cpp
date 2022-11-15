#include "FileService.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace VlkEngine {

	std::vector<char> FileService::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			std::cout << filename;
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}


	stbi_uc* FileService::LoadAssetImage(const std::string& filename, int* texWidth, int* texHeight, int* texChannels, const int STBI_type)
	{
		stbi_uc* pixels = stbi_load(&filename[0], texWidth, texHeight, texChannels, STBI_type);
		return pixels;
	}

	void FileService::CleanUpAssetImage(stbi_uc* pixels)
	{
		stbi_image_free(pixels);
	}
}