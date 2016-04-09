#ifndef __ALIGNED_CLASS_H_
#define __ALIGNED_CLASS_H_

template<size_t Alignment>
class AlignedClass
{
public:
	static void* operator new(size_t size) {
		return _aligned_malloc(size,Alignment);
	}

	static void operator delete(void* memory) {
		_aligned_free(memory);
	}
};

#endif