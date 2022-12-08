#include "Actor.h"


namespace VlkEngine {
	
	void Actor::AddComponent(Component* component)
	{
		components.push_back(component);
	}

	void Actor::SetRenderComponent(RenderComponent* rendercomponent)
	{
		renderComponent = rendercomponent;
	}
}