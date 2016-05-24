#include "Engine/EventSystem.h"

namespace StenGine
{

void EventSystem::Update()
{
	InvokeAll(m_onUpdateInputHandlers);
	InvokeAll(m_onUpdateAnimationHandlers);
	InvokeAll(m_onUpdatePhysicsHandlers);
	InvokeAll(m_onUpdateHandlers);
	InvokeAll(m_onPreRenderHandlers);
	InvokeAll(m_onRenderHandlers);
	InvokeAll(m_onPostRenderHandlers);
}

void EventSystem::RegisterEventHandler(EventType type, EventHandler handler)
{
	switch (type)
	{
	case EventType::UPDATE_INPUT:
		m_onUpdateInputHandlers.push_back(handler);
		break;
	case EventType::UPDATE_ANIMATION:
		m_onUpdateAnimationHandlers.push_back(handler);
		break;
	case EventType::UPDATE_PHYSICS:
		m_onUpdatePhysicsHandlers.push_back(handler);
		break;
	case EventType::UPDATE:
		m_onUpdateHandlers.push_back(handler);
		break;
	case EventType::PRE_RENDER:
		m_onPreRenderHandlers.push_back(handler);
		break;
	case EventType::RENDER:
		m_onRenderHandlers.push_back(handler);
		break;
	case EventType::POST_RENDER:
		m_onPostRenderHandlers.push_back(handler);
		break;
	}
}


DEFINE_SINGLETON_CLASS(EventSystem)

}