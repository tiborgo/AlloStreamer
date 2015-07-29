#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

#include "CubemapSource.hpp"
#include "Source.hpp"

class CubemapVideoSource : public VideoSource {
public:
    class CubemapStereoFrame : public Frame {
    public:

        class CubemapPixelData : public PixelData {
        public:
            CubemapPixelData(CubemapFace* face) : face_(face) { }

            virtual int width() {
                return face_->getContent()->getWidth();
            }
            virtual int height() {
                return face_->getContent()->getHeight();
            }
            virtual int stride() {
                ::AVPixelFormat fmt = face_->getContent()->getFormat();
                if(fmt == AV_PIX_FMT_RGB24)
                {
                    return face_->getContent()->getWidth() * 3;
                }
                else if(fmt == AV_PIX_FMT_RGBA)
                {
                    return face_->getContent()->getWidth() * 4;
                }
                else
                {
                    return 0;
                }
            }
            virtual void* pixels() {
                return face_->getContent()->getPixels();
            }

            CubemapFace* face_;

            virtual ~CubemapPixelData() { }
        };

        CubemapStereoFrame(StereoCubemap* cubemap) : cubemap_(cubemap) {
            Cubemap* left = cubemap_->getEye(0);
            Cubemap* right = cubemap_->getEye(1);
            if(cubemap_->getEyesCount() == 2) {
                faces[0][0] = new CubemapPixelData(left->getFace(0));
                faces[0][1] = new CubemapPixelData(left->getFace(1));
                faces[0][2] = new CubemapPixelData(left->getFace(2));
                faces[0][3] = new CubemapPixelData(left->getFace(3));
                faces[0][4] = new CubemapPixelData(left->getFace(4));
                faces[0][5] = new CubemapPixelData(left->getFace(5));
                faces[1][0] = new CubemapPixelData(right->getFace(0));
                faces[1][1] = new CubemapPixelData(right->getFace(1));
                faces[1][2] = new CubemapPixelData(right->getFace(2));
                faces[1][3] = new CubemapPixelData(right->getFace(3));
                faces[1][4] = new CubemapPixelData(right->getFace(4));
                faces[1][5] = new CubemapPixelData(right->getFace(5));
            } else {
                faces[0][0] = new CubemapPixelData(left->getFace(0));
                faces[0][1] = new CubemapPixelData(left->getFace(1));
                faces[0][2] = new CubemapPixelData(left->getFace(2));
                faces[0][3] = new CubemapPixelData(left->getFace(3));
                faces[0][4] = new CubemapPixelData(left->getFace(4));
                faces[0][5] = new CubemapPixelData(left->getFace(5));
                faces[1][0] = new CubemapPixelData(left->getFace(0));
                faces[1][1] = new CubemapPixelData(left->getFace(1));
                faces[1][2] = new CubemapPixelData(left->getFace(2));
                faces[1][3] = new CubemapPixelData(left->getFace(3));
                faces[1][4] = new CubemapPixelData(left->getFace(4));
                faces[1][5] = new CubemapPixelData(left->getFace(5));
            }
        }

        virtual PixelData* getSubImage(int id, int eye = 0) {
            return faces[eye][id];
        }

        virtual int getSubImageCount() {
			int count = 0;
			for (int i = 0; cubemap_->getEyesCount(); i++)
			{
				count += cubemap_->getEye(i)->getFacesCount();
			}
			return count;
		}
        virtual bool isStereo() { return true; }

        virtual ~CubemapStereoFrame() {
            StereoCubemap::destroy(cubemap_);
        }
        StereoCubemap* cubemap_;
        CubemapPixelData* faces[StereoCubemap::MAX_EYES_COUNT][Cubemap::MAX_FACES_COUNT];
    };


    CubemapVideoSource(const char* url, const CreateFlags& flags) {
        ::AVPixelFormat avpf;
        switch(flags.pixel_format) {
            case kPixelFormat_RGB24: avpf = AV_PIX_FMT_RGB24; break;
        }
        source_ = CubemapSource::createFromRTSP(url, flags.resolution, avpf);
        cached_frame_ = 0;
        
        std::function<void (CubemapSource*, StereoCubemap*)> callback = boost::bind(&CubemapVideoSource::onNextCubemap,
                                                                                    this,
                                                                                    _1,
                                                                                    _2);
        source_->setOnNextCubemap(callback);
    }

    void onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
    {
        CubemapStereoFrame* new_frame_ = new CubemapStereoFrame(cubemap);
        
        lockFrame();
        if(cached_frame_) delete cached_frame_;
        cached_frame_ = new_frame_;
        unlockFrame();
    }
    
    virtual Frame* getCurrentFrame() {
        return cached_frame_;
    }
    virtual void lockFrame() {
        mutex.lock();
    }
    virtual void unlockFrame() {
        mutex.unlock();
    }
    virtual bool nextFrame() {
        lockFrame();
        bool result;
        if (cached_frame_) {
            result = true;
        }
        else {
            result = false;
        }
        unlockFrame();

        return result;
    }

    ~CubemapVideoSource() {
        CubemapSource::destroy(source_);
    }

    CubemapStereoFrame* cached_frame_;
    CubemapSource* source_;

    boost::mutex mutex;
};

VideoSource::~VideoSource() { }
VideoSource::Frame::~Frame() { }
VideoSource::PixelData::~PixelData() { }

VideoSource* VideoSource::CreateFromRTSP(const char* url, const CreateFlags& flags) {
    return new CubemapVideoSource(url, flags);
}
