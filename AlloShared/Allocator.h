#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/segment_manager.hpp>

class Allocator
{
public:
    virtual void* allocate(size_t bytes) = 0;
    virtual void deallocate(void* object, size_t size) = 0;
};

class HeapAllocator : public Allocator
{
public:
    void* allocate(size_t bytes);
    void deallocate(void* object, size_t size);
};

class ShmAllocator : public Allocator
{
public:
    typedef boost::interprocess::allocator<boost::uint8_t,
        boost::interprocess::managed_shared_memory::segment_manager> BoostShmAllocator;
    
    ShmAllocator(BoostShmAllocator& allocator);
    void* allocate(size_t bytes);
    void deallocate(void* object, size_t size);
    
private:
    BoostShmAllocator& allocator;
};
