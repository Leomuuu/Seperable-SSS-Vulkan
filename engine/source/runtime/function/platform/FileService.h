#pragma once
#include "FileHeader.h"
#include <stb_image.h>

namespace VlkEngine {
	class FileService {
		public:
			static std::vector<char> ReadFile(const std::string& filename);

			static stbi_uc* LoadAssetImage(const std::string& filename,
				int* texWidth, int* texHeight, int* texChannels, const int STBI_type);
			static void CleanUpAssetImage(stbi_uc* pixels);
	};
}