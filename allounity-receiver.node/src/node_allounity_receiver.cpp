#include "AlloReceiver/Wrappers.hpp"

#include <nan.h>
#include <node_buffer.h>

using namespace v8;

class NODE_VideoSource : public Nan::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports) {
        Nan::HandleScope scope;

        // New({ width:, height:, active_stereo:, fullscreen:, title:, config: (config rules all) }
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New<v8::String>("RTSPCubemapSource").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // Prototype.

        Nan::SetPrototypeMethod(tpl, "pollNextCubemap", NODE_pollNextCubemap);
        Nan::SetPrototypeMethod(tpl, "getCurrentCubemapPixels", NODE_getCurrentCubemapPixels);
        Nan::SetPrototypeMethod(tpl, "getCubemapResolution", NODE_getCubemapResolution);
        Nan::SetPrototypeMethod(tpl, "isStereo", NODE_isStereo);
        Nan::SetPrototypeMethod(tpl, "getPixelFormat", NODE_getPixelFormat);

        constructor.Reset(tpl->GetFunction());

        // Export constructor.
        Nan::Set(exports, Nan::New<v8::String>("RTSPCubemapSource").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
    }

private:
    explicit NODE_VideoSource(RTSPCubemapSourceWrapper* client_) {
        client = client_;
    }
    ~NODE_VideoSource() {
        RTSPCubemapSourceWrapper::Destroy(client);
    }

    RTSPCubemapSourceWrapper* client;

    static NAN_METHOD(New);
    static NAN_METHOD(NODE_pollNextCubemap);
    static NAN_METHOD(NODE_getCurrentCubemapPixels);
    static NAN_METHOD(NODE_getCubemapResolution);
    static NAN_METHOD(NODE_isStereo);
    static NAN_METHOD(NODE_getPixelFormat);

    static Nan::Persistent<v8::Function> constructor;
};

Nan::Persistent<v8::Function> NODE_VideoSource::constructor;

NAN_METHOD(NODE_VideoSource::New) {
    Nan::HandleScope scope;
    if (info.IsConstructCall()) {
        Nan::Utf8String url(info[0]);

        RTSPCubemapSourceWrapper::CreateOptions options;
        std::string interface("0.0.0.0");

        if(info[1]->IsObject()) {
            Handle<Object> obj = info[1]->ToObject();
            if(!obj->Get(Nan::New<String>("pixel_format").ToLocalChecked())->IsUndefined()) {
                Nan::Utf8String pixel_format(obj->Get(Nan::New<String>("pixel_format").ToLocalChecked()));
                std::string pfmt = *pixel_format;
                if(pfmt == "rgba32") {
                    options.pixel_format = RTSPCubemapSourceWrapper::kRGBA32_PixelFormat;
                }
                if(pfmt == "yuv420p") {
                    options.pixel_format = RTSPCubemapSourceWrapper::kYUV420P_PixelFormat;
                }
            }
            if(!obj->Get(Nan::New<String>("interface").ToLocalChecked())->IsUndefined()) {
                Nan::Utf8String interface_str(obj->Get(Nan::New<String>("interface").ToLocalChecked()));
                interface = *interface_str;
            }
        }
        RTSPCubemapSourceWrapper* client = RTSPCubemapSourceWrapper::Create(*url, interface.c_str(), options);
        NODE_VideoSource* obj = new NODE_VideoSource(client);
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        // info.GetReturnValue().Set(Nan::New<v8::Undefined>());
    }
}

namespace {
    void do_nothing_callback2(char*, void*) { }
}

NAN_METHOD(NODE_VideoSource::NODE_pollNextCubemap) {
    NODE_VideoSource* obj = ObjectWrap::Unwrap<NODE_VideoSource>(info.This());
    info.GetReturnValue().Set(Nan::New<v8::Boolean>(obj->client->pollNextCubemap()));
}
NAN_METHOD(NODE_VideoSource::NODE_getCurrentCubemapPixels) {
    NODE_VideoSource* obj = ObjectWrap::Unwrap<NODE_VideoSource>(info.This());
    int face = info[0]->IntegerValue();
    int eye = info[1]->IsUndefined() ? 0 : info[1]->IntegerValue();
    void* pixels = obj->client->getCurrentCubemapPixels(face, eye);
    if(!pixels) return;

    int resolution = obj->client->getCubemapResolution();
    int pixel_size = 0;
    if(obj->client->getPixelFormat() == RTSPCubemapSourceWrapper::kRGBA32_PixelFormat) pixel_size = 4;
    if(obj->client->getPixelFormat() == RTSPCubemapSourceWrapper::kYUV420P_PixelFormat) pixel_size = 3;

    info.GetReturnValue().Set(Nan::NewBuffer((char*)pixels, resolution * resolution * pixel_size, do_nothing_callback2, NULL).ToLocalChecked());
}
NAN_METHOD(NODE_VideoSource::NODE_getCubemapResolution) {
    NODE_VideoSource* obj = ObjectWrap::Unwrap<NODE_VideoSource>(info.This());
    info.GetReturnValue().Set(Nan::New<v8::Integer>(obj->client->getCubemapResolution()));
}
NAN_METHOD(NODE_VideoSource::NODE_isStereo) {
    NODE_VideoSource* obj = ObjectWrap::Unwrap<NODE_VideoSource>(info.This());
    info.GetReturnValue().Set(Nan::New<v8::Boolean>(obj->client->isStereo()));
}
NAN_METHOD(NODE_VideoSource::NODE_getPixelFormat) {
    NODE_VideoSource* obj = ObjectWrap::Unwrap<NODE_VideoSource>(info.This());
    switch(obj->client->getPixelFormat()) {
        case RTSPCubemapSourceWrapper::kRGBA32_PixelFormat: {
            info.GetReturnValue().Set(Nan::New<v8::String>("rgba32").ToLocalChecked());
        }
        case RTSPCubemapSourceWrapper::kYUV420P_PixelFormat: {
            info.GetReturnValue().Set(Nan::New<v8::String>("yuv420p").ToLocalChecked());
        }
    }
}


NAN_MODULE_INIT(NODE_init) {
    Nan::HandleScope();
    NODE_VideoSource::Init(target);
}

NODE_MODULE(allounity_receiver, NODE_init);
