#pragma once
#include "System/SingletonClass.h"
#include <unordered_set>
#include <vector>
#include <functional>

namespace StenGine
{

class EventSystem : public SingletonClass<EventSystem>
{
public:
	using EventHandler = std::function<void()>;

	enum class EventType
	{
		UPDATE_INPUT,
		UPDATE_ANIMATION,
		UPDATE_PHYSICS,
		UPDATE,
		UPDATE_TRANSFORM,
		PRE_RENDER,
		RENDER,
		POST_RENDER,
	};

	void Update();
	void RegisterEventHandler(EventType, EventHandler);
	//void UnregisterEventHandler(EventType, EventHandler);

private:
	using EventHandlerPool = std::vector<EventHandler>;

	inline void InvokeAll(EventHandlerPool& pool)
	{
		for (auto &handler : pool)
		{
			handler();
		}
	}

	EventHandlerPool m_onUpdateInputHandlers;
	EventHandlerPool m_onUpdateAnimationHandlers;
	EventHandlerPool m_onUpdatePhysicsHandlers;
	EventHandlerPool m_onUpdateHandlers;
	EventHandlerPool m_onUpdateTransformHandlers;
	EventHandlerPool m_onPreRenderHandlers;
	EventHandlerPool m_onRenderHandlers;
	EventHandlerPool m_onPostRenderHandlers;
};

}