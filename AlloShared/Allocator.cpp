#include "Allocator.h"

void* HeapAllocator::allocate(size_t bytes)
{
    std::allocator<boost::uint8_t> allocator;
    return allocator.allocate(bytes);
}

void HeapAllocator::deallocate(void* object, size_t size)
{
    std::allocator<boost::uint8_t> allocator;
    return allocator.deallocate((boost::uint8_t*)object, size);
}

ShmAllocator::ShmAllocator(ShmAllocator::BoostShmAllocator& allocator)
    :
    allocator(allocator)
{
}

void* ShmAllocator::allocate(size_t bytes)
{
    return allocator.allocate(bytes).get();
}

void ShmAllocator::deallocate(void* object, size_t size)
{
    return allocator.deallocate((boost::uint8_t*)object, size);
}