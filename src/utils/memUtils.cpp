#include "memUtils.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#include <malloc.h>
#endif

void* MemoryUtils::alignedAlloc(size_t size, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        throw std::invalid_argument("Alignment must be a power of 2");
    }
    
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    int result = posix_memalign(&ptr, alignment, size);
    if (result != 0) {
        throw std::bad_alloc();
    }
    return ptr;
#endif
}

void MemoryUtils::alignedFree(void* ptr) {
    if (ptr == nullptr) return;
    
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void MemoryUtils::fastMemcpy(void* dest, const void* src, size_t size) {
#ifdef SIMD_OPTIMIZED
    if (size >= 32 && isAligned(dest, 32) && isAligned(src, 32)) {
        simdMemcpy(dest, src, size);
        return;
    }
#endif
    std::memcpy(dest, src, size);
}

void MemoryUtils::fastMemset(void* dest, int value, size_t size) {
#ifdef SIMD_OPTIMIZED
    if (size >= 32 && isAligned(dest, 32)) {
        simdMemset(dest, value, size);
        return;
    }
#endif
    std::memset(dest, value, size);
}

int MemoryUtils::fastMemcmp(const void* ptr1, const void* ptr2, size_t size) {
    return std::memcmp(ptr1, ptr2, size);
}

bool MemoryUtils::isAligned(const void* ptr, size_t alignment) {
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

size_t MemoryUtils::getAlignment(const void* ptr) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    if (addr == 0) return SIZE_MAX;
    
    size_t alignment = 1;
    while ((addr & 1) == 0) {
        addr >>= 1;
        alignment <<= 1;
    }
    return alignment;
}

void MemoryUtils::simdMemcpy(void* dest, const void* src, size_t size) {
#ifdef SIMD_OPTIMIZED
    const uint8_t* srcBytes = static_cast<const uint8_t*>(src);
    uint8_t* destBytes = static_cast<uint8_t*>(dest);
    
    size_t simdSize = size & ~31;
    for (size_t i = 0; i < simdSize; i += 32) {
        __m256i data = _mm256_load_si256(reinterpret_cast<const __m256i*>(srcBytes + i));
        _mm256_store_si256(reinterpret_cast<__m256i*>(destBytes + i), data);
    }
    
    if (size > simdSize) {
        std::memcpy(destBytes + simdSize, srcBytes + simdSize, size - simdSize);
    }
#else
    std::memcpy(dest, src, size);
#endif
}

void MemoryUtils::simdMemset(void* dest, int value, size_t size) {
#ifdef SIMD_OPTIMIZED
    uint8_t* destBytes = static_cast<uint8_t*>(dest);
    __m256i valueVec = _mm256_set1_epi8(static_cast<char>(value));
    
    size_t simdSize = size & ~31;
    for (size_t i = 0; i < simdSize; i += 32) {
        _mm256_store_si256(reinterpret_cast<__m256i*>(destBytes + i), valueVec);
    }
    
    if (size > simdSize) {
        std::memset(destBytes + simdSize, value, size - simdSize);
    }
#else
    std::memset(dest, value, size);
#endif
}