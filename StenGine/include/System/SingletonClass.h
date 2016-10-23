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

	static bool Created()
	{
		return _instance != nullptr;
	}
};


#define DEFINE_SINGLETON_CLASS(T) \
	T* T::_instance = nullptr;


template <class T>
class AbstractSingletonClass
{
protected:
	static T* _instance;

public:

	static bool Created()
	{
		return _instance != nullptr;
	}
};

#define DECLARE_ABSTRACT_SINGLETON_CLASS(BASE) \
	static BASE* _instance; \
	static BASE* Instance();\


#define DEFINE_ABSTRACT_SINGLETON_CLASS(BASE, DERIVED) \
	BASE* BASE::_instance = nullptr;	\
										\
	BASE* BASE::Instance()		\
	{									\
		if (!_instance)					\
			_instance = new DERIVED();  \
		return _instance;				\
	}									\

}
