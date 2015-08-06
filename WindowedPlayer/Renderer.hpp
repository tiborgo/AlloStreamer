#pragma once

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/frame.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/time.h>
    #include <libswscale/swscale.h>
}
#include <boost/thread.hpp>
#include "AlloReceiver/AlloReceiver.h"

class Renderer
{
public:
    Renderer(CubemapSource* cubemapSource);
    virtual ~Renderer();
    
	void start();

    void setOnDisplayedFrame(std::function<void (Renderer*)>& callback);
    void setOnDisplayedCubemapFace(std::function<void (Renderer*, int)>& callback);
    
protected:
    std::function<void (Renderer*)> onDisplayedFrame;
    std::function<void (Renderer*, int)> onDisplayedCubemapFace;

private:
	void onNextCubemap(CubemapSource* source, StereoCubemap* cubemap);

	std::vector<AVFrame*> currentFrames;
	SwsContext* resizeCtx;
	CubemapSource* cubemapSource;
	StereoCubemap* cubemap;
	boost::mutex nextCubemapMutex;
	bool newCubemap;
};
