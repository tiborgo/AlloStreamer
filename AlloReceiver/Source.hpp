// This is a independent generic source class.

class VideoSource {
public:
    enum PixelFormat {
        kPixelFormat_RGB24 = 0,
    };

    enum FrameType {
        kFrameType_Cubemap = 0,
    };

    class PixelData {
    public:
        virtual int width() = 0;
        virtual int height() = 0;
        virtual int stride() = 0;
        virtual void* pixels() = 0;

    protected:
        virtual ~PixelData();
    };

    // Represent a frame.
    class Frame {
    public:
        virtual PixelData* getSubImage(int index, int eye = 0) = 0;
        virtual bool isStereo() = 0;
        virtual int getSubImageCount() = 0;

    protected:
        virtual ~Frame();
    };

    // Get the current frame.
    // If there's currently no frame, this returns null.
    virtual Frame* getCurrentFrame() = 0;
    // Move to the next frame, return true if frame updated.
    virtual bool nextFrame() = 0;
    virtual void lockFrame() = 0;
    virtual void unlockFrame() = 0;

    struct CreateFlags {
        int resolution;
        PixelFormat pixel_format;
        FrameType frame_type;
		unsigned long buffer_size;
        CreateFlags() {
            resolution = 2048;
            pixel_format = kPixelFormat_RGB24;
            frame_type = kFrameType_Cubemap;
			buffer_size = 200000000;
        }
    };

    static VideoSource* CreateFromRTSP(const char* url, const CreateFlags& flags);
    static void Destroy(VideoSource* source);
protected:
        virtual ~VideoSource();
};
