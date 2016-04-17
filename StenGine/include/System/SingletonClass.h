#pragma once

namespace StenGine
{
template <class T>
class SingletonClass
{
protected:
	static T* _instance;

public:
	static T* Instance() {
		if (!_instance)
			_instance = new T();
		return _instance;
	}
};

}

#define DEFINE_SINGLETON_CLASS(T) \
	T* T::_instance = nullptr;