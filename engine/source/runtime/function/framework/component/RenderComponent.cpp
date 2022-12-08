#include "RenderComponent.h"

namespace VlkEngine {
	
	void RenderComponent::SetModel(std::string modelpath)
	{
		modelPath = modelpath;
	}

	void RenderComponent::SetTexture(std::string colortexturePath, std::string cavitytexturePath, std::string gosstexturePath, std::string speculartexturePath, std::string micronormaltexturePath, std::string normaltexturePath, std::string translucencytexturePath)
	{
		colorTexturePath = colortexturePath;
		cavityTexturePath = cavitytexturePath;
		gossTexturePath = gosstexturePath;
		specularTexturePath = speculartexturePath;
		micronormalTexturePath = micronormaltexturePath;
		normalTexturePath = normaltexturePath;
		translucencyTexturePath = translucencytexturePath;
	}




	
}