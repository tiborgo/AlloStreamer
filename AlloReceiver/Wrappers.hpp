// Clean wrapper to the cubemap source.
// This file should only use C/C++ language features, it should not include anything.

#ifndef AlloReceiverWrapper_HPP_INCLUDED
#define AlloReceiverWrapper_HPP_INCLUDED

class RTSPCubemapSourceWrapper {
public:
    // Pixel formats.
    typedef int PixelFormat;
    static const PixelFormat kRGBA32_PixelFormat  = 0;
    static const PixelFormat kYUV420P_PixelFormat  = 1;

    typedef int CubemapFace;
    static const int kPOSITIVE_X_CubemapFace = 0;
    static const int kNEGATIVE_X_CubemapFace = 1;
    static const int kPOSITIVE_Y_CubemapFace = 2;
    static const int kNEGATIVE_Y_CubemapFace = 3;
    static const int kPOSITIVE_Z_CubemapFace = 4;
    static const int kNEGATIVE_Z_CubemapFace = 5;

    typedef int Eye;
    static const int kLeft_Eye = 0;
    static const int kRight_Eye = 1;

    // All functions in this class is non-blocking.

    // Poll for the next cubemap, if available, switch to the next cubemap and return true, otherwise return false.
    virtual bool pollNextCubemap() = 0;

    // Get pixels for a cubemap face at an eye.
    virtual void* getCurrentCubemapPixels(CubemapFace face, Eye eye) = 0;
    virtual int getCurrentCubemapByteSize(CubemapFace face, Eye eye) = 0;

    // Return the cubemap pixel format.
    virtual PixelFormat getPixelFormat() = 0;

    struct CreateOptions {
        PixelFormat pixel_format;
        int sink_buffer_size;

        CreateOptions() : pixel_format(kYUV420P_PixelFormat), sink_buffer_size(-1) {
        }
    };

    // Constructors.
    static RTSPCubemapSourceWrapper* Create(const char* address, const char* interface = "0.0.0.0", const CreateOptions& options = CreateOptions());
    static void Destroy(RTSPCubemapSourceWrapper* object);
protected:
    virtual ~RTSPCubemapSourceWrapper();
};

#endif
