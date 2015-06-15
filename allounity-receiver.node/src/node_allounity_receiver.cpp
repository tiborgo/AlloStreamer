#include "AlloReceiver/Source.hpp"

#include <nan.h>
#include <node_buffer.h>

#include <uv.h>
#include <pthread.h>

using namespace v8;

class NODE_VideoSource : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports) {
        NanScope();

        // New({ width:, height:, active_stereo:, fullscreen:, title:, config: (config rules all) }
        v8::Local<v8::FunctionTemplate> tpl = NanNew<v8::FunctionTemplate>(New);
        tpl->SetClassName(NanNew<v8::String>("VideoSource"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // Prototype.
        NODE_SET_PROTOTYPE_METHOD(tpl, "getCurrentFrame", NODE_getCurrentFrame);
        NODE_SET_PROTOTYPE_METHOD(tpl, "onFrame", NODE_onFrame);
        NODE_SET_PROTOTYPE_METHOD(tpl, "stop", NODE_stop);

        NanAssignPersistent(constructor, tpl->GetFunction());

        // Export constructor.
        exports->Set(NanNew<v8::String>("VideoSource"), tpl->GetFunction());
    }

private:
    explicit NODE_VideoSource(const char* url, const VideoSource::CreateFlags& flags)
    : url_(url), flags_(flags), should_exit_(false) {

        source_ = nullptr;

        loop_ = uv_default_loop();
        uv_async_init(loop_, &async_, PostMessage);
        async_.data = this;
        uv_mutex_init(&mutex_);

        pthread_create(&thread_, 0, WorkerThread, this);
    }
    ~NODE_VideoSource() {
        stop();
        uv_unref((uv_handle_t*)&async_);
        uv_mutex_destroy(&mutex_);
        NanDisposePersistent(on_frame_callback_);
    }

    std::string url_;
    VideoSource::CreateFlags flags_;
    VideoSource* source_;
    Persistent<Function> on_frame_callback_;
    bool should_exit_;
    pthread_t thread_;

    uv_loop_t *loop_;
    uv_async_t async_;
    uv_mutex_t mutex_;

    static void* WorkerThread(void* userdata) {
        NODE_VideoSource* self = (NODE_VideoSource*)userdata;
        self->workerThread();
        return 0;
    }

    void stop() {
        if(!should_exit_) {
            should_exit_ = true;
            pthread_join(thread_, 0);
        }
    }

    void workerThread() {
        try {
            source_ = VideoSource::CreateFromRTSP(url_.c_str(), flags_);
            while(!should_exit_) {
                bool result = source_->nextFrame();
                if(result) {
                    uv_async_send(&async_);
                }
            }
            VideoSource::Destroy(source_);
        } catch(...) {
        }
    }

    static void PostMessage(uv_async_t *handle) {
        NODE_VideoSource* self = (NODE_VideoSource*)handle->data;

        const int argc = 0;
        Handle<Value> argv[argc] = { };
        self->source_->lockFrame();
        NanNew(self->on_frame_callback_)->Call(NanObjectWrapHandle(self), argc, argv);
        self->source_->unlockFrame();
    }

    static NAN_METHOD(New);

    static NAN_METHOD(NODE_onFrame);
    static NAN_METHOD(NODE_getCurrentFrame);
    static NAN_METHOD(NODE_stop);

    static v8::Persistent<v8::Function> constructor;
};

v8::Persistent<v8::Function> NODE_VideoSource::constructor;

NAN_METHOD(NODE_VideoSource::New) {
    NanScope();
    if (args.IsConstructCall()) {
        NanUtf8String url(args[0]);
        VideoSource::CreateFlags flags;

        if(args[1]->IsObject()) {
            Handle<Object> obj = args[1]->ToObject();
            if(!obj->Get(NanNew<String>("pixel_format"))->IsUndefined()) {
                flags.pixel_format = (VideoSource::PixelFormat)obj->Get(NanNew<String>("pixel_format"))->IntegerValue();
            }
            if(!obj->Get(NanNew<String>("frame_type"))->IsUndefined()) {
                flags.frame_type = (VideoSource::FrameType)obj->Get(NanNew<String>("frame_type"))->IntegerValue();
            }
            if(!obj->Get(NanNew<String>("resolution"))->IsUndefined()) {
                flags.resolution = obj->Get(NanNew<String>("resolution"))->IntegerValue();
            }
        }

        NODE_VideoSource* obj = new NODE_VideoSource(*url, flags);
        obj->Wrap(args.This());
        NanReturnValue(args.This());
    }
    else {
        v8::Local<v8::Function> cons = NanNew<v8::Function>(constructor);
        NanReturnValue(cons->NewInstance());
    }
}

NAN_METHOD(NODE_VideoSource::NODE_onFrame) {
    NanScope();
    NODE_VideoSource* obj = node::ObjectWrap::Unwrap<NODE_VideoSource>(args.This());
    if(!args[0]->IsFunction()) {
        NanThrowError("VideoSource::onFrame: callback should be function.");
    }
    NanAssignPersistent(obj->on_frame_callback_, args[0].As<Function>());
    NanReturnUndefined();
}

namespace {
    void do_nothing_free_callback(char* data, void* hint) { }
}

NAN_METHOD(NODE_VideoSource::NODE_getCurrentFrame) {
    NanScope();
    NODE_VideoSource* obj = node::ObjectWrap::Unwrap<NODE_VideoSource>(args.This());
    if(!obj->source_) NanReturnUndefined();

    VideoSource::Frame* frame = obj->source_->getCurrentFrame();

    if(!frame) NanReturnUndefined();

    int subimage_id = args[0]->IntegerValue();
    int eye_id = args[1]->IntegerValue();

    VideoSource::PixelData* pixels = frame->getSubImage(subimage_id, eye_id);

    Handle<Value> buffer = NanNewBufferHandle((char*)pixels->pixels(), pixels->height() * pixels->stride() * 3, do_nothing_free_callback, NULL);
    Handle<Object> ret = NanNew<Object>();

    ret->Set(NanNew<String>("pixels"), buffer);
    ret->Set(NanNew<String>("width"), NanNew<Integer>(pixels->width()));
    ret->Set(NanNew<String>("height"), NanNew<Integer>(pixels->height()));
    ret->Set(NanNew<String>("stride"), NanNew<Integer>(pixels->stride()));

    NanReturnValue(ret);
}

NAN_METHOD(NODE_VideoSource::NODE_stop) {
    NanScope();
    NODE_VideoSource* obj = node::ObjectWrap::Unwrap<NODE_VideoSource>(args.This());
    obj->stop();
    NanReturnUndefined();
}

void NODE_init(v8::Handle<v8::Object> exports) {
    NODE_VideoSource::Init(exports);
    exports->Set(NanNew<String>("kPixelFormat_RGB24"), NanNew<Uint32>((int32_t)VideoSource::kPixelFormat_RGB24));
    exports->Set(NanNew<String>("kFrameType_Cubemap"), NanNew<Uint32>((int32_t)VideoSource::kFrameType_Cubemap));
}


NODE_MODULE(allounity_receiver, NODE_init);
