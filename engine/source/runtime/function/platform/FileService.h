#include "FileHeader.h"

namespace VlkEngine {
	class FileService {
		public:
			static std::vector<char> ReadFile(const std::string& filename);

	};
}