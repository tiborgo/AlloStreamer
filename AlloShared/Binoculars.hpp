#pragma once

#include "Frame.hpp"

class Binoculars
{

public:
	typedef boost::interprocess::offset_ptr<Binoculars> Ptr;
    
    Frame* getContent();
    
    static Binoculars* create(Frame* content,
                              Allocator& allocator);
    static void destroy(Binoculars* cubemapFace);
    
protected:
    Binoculars(Frame* content,
               Allocator& allocator);
    ~Binoculars();
    
    Frame::Ptr content;
    Allocator& allocator;
};