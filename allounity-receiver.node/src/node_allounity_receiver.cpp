#include "AlloReceiver/Source.hpp"

#include <nan.h>
#include <node_buffer.h>

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
        NODE_SET_PROTOTYPE_METHOD(tpl, "nextFrame", NODE_nextFrame);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getCurrentFrame", NODE_getCurrentFrame);

        NanAssignPersistent(constructor, tpl->GetFunction());

        // Export constructor.
        exports->Set(NanNew<v8::String>("VideoSource"), tpl->GetFunction());
    }

private:
    explicit NODE_VideoSource(VideoSource* source) : source_(source) {

    }
    ~NODE_VideoSource() {
        VideoSource::Destroy(source_);
    }

    VideoSource* source_;

    static NAN_METHOD(New);

    static NAN_METHOD(NODE_nextFrame);
    static NAN_METHOD(NODE_getCurrentFrame);

    static v8::Persistent<v8::Function> constructor;
};

v8::Persistent<v8::Function> NODE_VideoSource::constructor;

NAN_METHOD(NODE_VideoSource::New) {
    NanScope();
    if (args.IsConstructCall()) {
        NanUtf8String *url = new NanUtf8String(args[0]);
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

        VideoSource* source = VideoSource::CreateFromRTSP(**url, flags);

        NODE_VideoSource* obj = new NODE_VideoSource(source);
        obj->Wrap(args.This());
        NanReturnValue(args.This());
    }
    else {
        v8::Local<v8::Function> cons = NanNew<v8::Function>(constructor);
        NanReturnValue(cons->NewInstance());
    }
}

NAN_METHOD(NODE_VideoSource::NODE_nextFrame) {
    NanScope();
    NODE_VideoSource* obj = node::ObjectWrap::Unwrap<NODE_VideoSource>(args.This());
    NanReturnValue(NanNew<Boolean>(obj->source_->nextFrame()));
}

namespace {
    void do_nothing_free_callback(char* data, void* hint) { }
}

NAN_METHOD(NODE_VideoSource::NODE_getCurrentFrame) {
    NanScope();
    NODE_VideoSource* obj = node::ObjectWrap::Unwrap<NODE_VideoSource>(args.This());

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

void NODE_init(v8::Handle<v8::Object> exports) {
    NODE_VideoSource::Init(exports);
    exports->Set(NanNew<String>("kPixelFormat_RGB24"), NanNew<Uint32>((int32_t)VideoSource::kPixelFormat_RGB24));
    exports->Set(NanNew<String>("kFrameType_Cubemap"), NanNew<Uint32>((int32_t)VideoSource::kFrameType_Cubemap));
}


NODE_MODULE(allounity_receiver, NODE_init);
