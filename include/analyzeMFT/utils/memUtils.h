#ifndef ANALYZEMFT_MEMUTILS_H
#define ANALYZEMFT_MEMUTILS_H

#include <cstddef>
#include <cstdint>
#include <memory>

#ifdef SIMD_OPTIMIZED
#include <immintrin.h>
#endif

class MemoryUtils {
public:
    static void* alignedAlloc(size_t size, size_t alignment);
    static void alignedFree(void* ptr);
    
    template<typename T>
    static std::unique_ptr<T, void(*)(void*)> makeAligned(size_t count = 1, size_t alignment = 32);
    
    static void fastMemcpy(void* dest, const void* src, size_t size);
    static void fastMemset(void* dest, int value, size_t size);
    static int fastMemcmp(const void* ptr1, const void* ptr2, size_t size);
    
    static bool isAligned(const void* ptr, size_t alignment);
    static size_t getAlignment(const void* ptr);
    
private:
    static void simdMemcpy(void* dest, const void* src, size_t size);
    static void simdMemset(void* dest, int value, size_t size);
};

template<typename T>
std::unique_ptr<T, void(*)(void*)> MemoryUtils::makeAligned(size_t count, size_t alignment) {
    T* ptr = static_cast<T*>(alignedAlloc(sizeof(T) * count, alignment));
    if (ptr) {
        for (size_t i = 0; i < count; ++i) {
            new(ptr + i) T();
        }
    }
    return std::unique_ptr<T, void(*)(void*)>(ptr, alignedFree);
}

#endif