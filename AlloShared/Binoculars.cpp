#include "Binoculars.hpp"

Binoculars::Binoculars(Frame* content,
                       Allocator& allocator)
    :
    content(content),
    allocator(allocator)
{

}

Binoculars::~Binoculars()
{
    Frame::destroy(content.get());
}

Frame* Binoculars::getContent()
{
    return content.get();
}

Binoculars* Binoculars::create(Frame* content,
                               Allocator& allocator)
{
    void* addr = allocator.allocate(sizeof(Binoculars));
    return new (addr) Binoculars(content, allocator);
}

void Binoculars::destroy(Binoculars* binoculars)
{
    binoculars->~Binoculars();
    binoculars->allocator.deallocate(binoculars, sizeof(Binoculars));
}