#pragma once

#include "AlloReceiver.h"
#include "AlloShared/Cubemap.hpp"

class ALLORECEIVER_API CubemapSource
{
public:
    typedef std::function<StereoCubemap* (CubemapSource*, StereoCubemap*)> OnNextCubemap;
    
	virtual void setOnNextCubemap(const OnNextCubemap& callback) = 0;
    //virtual void setOnDroppedNALU(std::function<void (CubemapSource*, int, uint8_t, size_t)>&   callback) = 0;
    //virtual void setOnAddedNALU  (std::function<void (CubemapSource*, int, uint8_t, size_t)>&   callback) = 0;
    
    static void destroy(CubemapSource* cubemapSource);
};
