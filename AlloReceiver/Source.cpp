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
                return face_->getWidth();
            }
            virtual int height() {
                return face_->getHeight();
            }
            virtual int stride() {
                ::AVPixelFormat fmt = face_->getFormat();
                if(fmt == AV_PIX_FMT_RGB24)
                    return face_->getWidth() * 3;
                if(fmt == AV_PIX_FMT_RGBA)
                    return face_->getWidth() * 4;
            }
            virtual void* pixels() {
                return face_->getPixels();
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

        virtual int getSubImageCount() { return 6; }
        virtual bool isStereo() { return true; }

        virtual ~CubemapStereoFrame() {
            StereoCubemap::destroy(cubemap_);
            for(int i = 0; i < 2; i++) {
                for(int j = 0; j < 6; j++) {
                    delete faces[i][j];
                }
            }
        }
        StereoCubemap* cubemap_;
        CubemapPixelData* faces[2][6];
    };


    CubemapVideoSource(const char* url, const CreateFlags& flags) {
        ::AVPixelFormat avpf;
        switch(flags.pixel_format) {
            case kPixelFormat_RGB24: avpf = AV_PIX_FMT_RGB24; break;
        }
        source_ = CubemapSource::createFromRTSP(url, flags.resolution, avpf);
        cached_frame_ = 0;
    }

    virtual Frame* getCurrentFrame() {
        return cached_frame_;
    }
    virtual bool nextFrame() {
        if(cached_frame_) delete cached_frame_;
        cached_frame_ = new CubemapStereoFrame(source_->getCurrentCubemap());
        return true;
    }

    ~CubemapVideoSource() {
        CubemapSource::destroy(source_);
    }

    CubemapStereoFrame* cached_frame_;
    CubemapSource* source_;
};

VideoSource::~VideoSource() { }
VideoSource::Frame::~Frame() { }
VideoSource::PixelData::~PixelData() { }

VideoSource* VideoSource::CreateFromRTSP(const char* url, const CreateFlags& flags) {
    return new CubemapVideoSource(url, flags);
}
