#pragma once
#include "../component/RenderComponent.h"

namespace VlkEngine {
	class Actor {
	public:
		RenderComponent* renderComponent;
		std::vector<Component*> components;

	public:
		void AddComponent(Component* component);
		void SetRenderComponent(RenderComponent* rendercomponent);

			

	};
}