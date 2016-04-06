// Clean wrapper to the cubemap source.
// This file should only use C/C++ language features, it should not include anything.

#include "RTSPCubemapSourceClient.hpp"
#include "Wrappers.hpp"

class RTSPCubemapSourceWrapper_Impl : public RTSPCubemapSourceWrapper {
public:
    RTSPCubemapSourceWrapper_Impl(const char* address, const char* interface, const CreateOptions& options) {
        pixel_format_ = options.pixel_format;

        const unsigned int DEFAULT_SINK_BUFFER_SIZE = 200000000;

        current_cubemap_ = nullptr;
        new_cubemap_ = nullptr;
        new_cubemap_available_ = false;

        switch(pixel_format_) {
            case kRGBA32_PixelFormat: {
                // std::cout << "Starting RTSPCubemapSourceClient with RGBA32 on " << address << std::endl;
                rtsp_client_ = RTSPCubemapSourceClient::create(address,
                    options.sink_buffer_size > 0 ? options.sink_buffer_size : DEFAULT_SINK_BUFFER_SIZE,
                    AV_PIX_FMT_RGBA, false, interface
                );
            } break;
            case kYUV420P_PixelFormat: {
                // std::cout << "Starting RTSPCubemapSourceClient with YUV420P on " << address << std::endl;
                rtsp_client_ = RTSPCubemapSourceClient::create(address,
                    options.sink_buffer_size > 0 ? options.sink_buffer_size : DEFAULT_SINK_BUFFER_SIZE,
                    AV_PIX_FMT_YUV420P, false, interface
                );
            } break;
        }

        rtsp_client_->setOnDidConnect(boost::bind(&RTSPCubemapSourceWrapper_Impl::onDidConnect, this, _1, _2));
        rtsp_client_->connect();
    }

    void onDidConnect(RTSPCubemapSourceClient* client, CubemapSource* cubemap_source) {
        cubemap_source_ = cubemap_source;
        cubemap_source_->setOnNextCubemap(boost::bind(&RTSPCubemapSourceWrapper_Impl::onNextCubemap, this, _1, _2));
    }

    StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap) {
        StereoCubemap* old_cubemap;
        boost::interprocess::scoped_lock<boost::mutex> lock(mutex);
        old_cubemap = new_cubemap_;
        new_cubemap_ = cubemap;
        new_cubemap_available_ = true;
        return old_cubemap;
    }

    // All functions in this class is non-blocking.

    // Poll for the next cubemap, if available, switch to the next cubemap and return true, otherwise return false.
    virtual bool pollNextCubemap() {
        boost::interprocess::scoped_lock<boost::mutex> lock(mutex);
        if(new_cubemap_available_) {
            StereoCubemap* tmp = new_cubemap_;
            new_cubemap_ = current_cubemap_;
            current_cubemap_ = tmp;
            new_cubemap_available_ = false;
            return true;
        } else {
            return false;
        }
    }

    // Get pixels for a cubemap face at an eye.
    virtual void* getCurrentCubemapPixels(CubemapFace face, Eye eye) {
        if(!current_cubemap_) return nullptr;

        if(eye >= current_cubemap_->getEyesCount()) return nullptr;
        auto cubemap_eye = current_cubemap_->getEye(eye);

        if(face >= cubemap_eye->getFacesCount()) return nullptr;
        auto cubemap_face = cubemap_eye->getFace(face);

        if(cubemap_face) return cubemap_face->getContent()->getPixels();
        return nullptr;
    }

    virtual int getCurrentCubemapByteSize(CubemapFace face, Eye eye) {
        if(!current_cubemap_) return 0;

        if(eye >= current_cubemap_->getEyesCount()) return 0;
        auto cubemap_eye = current_cubemap_->getEye(eye);

        if(face >= cubemap_eye->getFacesCount()) return 0;
        auto cubemap_face = cubemap_eye->getFace(face);

        if(cubemap_face) {
            int w = cubemap_face->getContent()->getWidth();
            int h = cubemap_face->getContent()->getHeight();
            int bit_per_pixel;
            if(cubemap_face->getContent()->getFormat() == AV_PIX_FMT_RGBA) {
                bit_per_pixel = 32;
            }
            if(cubemap_face->getContent()->getFormat() == AV_PIX_FMT_YUV420P) {
                bit_per_pixel = 12;
            }
            return w * h * bit_per_pixel / 8;
        }
        return 0;
    }

    // Get the resolution of the cubemap.
    virtual int getCubemapResolution() {
        return resolution_;
    }
    // Return true if the cubemap is stereo.
    virtual bool isStereo() {
        return is_stereo_;
    }
    // Return the cubemap pixel format.
    virtual PixelFormat getPixelFormat() {
        return pixel_format_;
    }

    virtual ~RTSPCubemapSourceWrapper_Impl() {
        CubemapSource::destroy(cubemap_source_);
        if(current_cubemap_) {
            StereoCubemap::destroy(current_cubemap_);
        }
        if(new_cubemap_) {
            StereoCubemap::destroy(new_cubemap_);
        }
        delete rtsp_client_;
        printf("Streamer stopped.\n");
    }

    int resolution_;
    bool is_stereo_;
    PixelFormat pixel_format_;
    RTSPCubemapSourceClient* rtsp_client_;
    CubemapSource* cubemap_source_;
    StereoCubemap* current_cubemap_;
    StereoCubemap* new_cubemap_;
    bool new_cubemap_available_;
    boost::mutex mutex;
};

RTSPCubemapSourceWrapper* RTSPCubemapSourceWrapper::Create(const char* address, const char* interface, const CreateOptions& options) {
    return new RTSPCubemapSourceWrapper_Impl(address, interface, options);
}

void RTSPCubemapSourceWrapper::Destroy(RTSPCubemapSourceWrapper* object) {
    delete object;
}

RTSPCubemapSourceWrapper::~RTSPCubemapSourceWrapper() { }
