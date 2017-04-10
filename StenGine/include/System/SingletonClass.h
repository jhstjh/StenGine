#pragma once

#include <assert.h>

namespace StenGine
{

template <class T>
class SingletonClass
{
protected:
	static T* _instance;

public:
	static T* Create()
	{
		assert(_instance == nullptr);
		_instance = new T();
		return _instance;
	}

	static T* Instance() 
	{
		assert(_instance != nullptr);
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
	static BASE* Create();\
	static BASE* Instance();\


#define DEFINE_ABSTRACT_SINGLETON_CLASS(BASE, DERIVED) \
	BASE* BASE::_instance = nullptr;	\
										\
	BASE* BASE::Create()				\
	{									\
		assert(_instance == nullptr);	\
		_instance = new DERIVED();		\
		return _instance;				\
	}									\
										\
	BASE* BASE::Instance()				\
	{									\
		assert(_instance != nullptr);	\
		return _instance;				\
	}									\

}
