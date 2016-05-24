#include "Graphics/Animation/Animation.h"
#include "Engine/EventSystem.h"

namespace StenGine
{

Animation::Animation()
{
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE_ANIMATION, [this]() {Update(); });
}

}