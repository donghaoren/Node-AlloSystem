#include <node.h>
#include <v8.h>

#include <graphics.h>

#include "node_graphics.h"

using namespace v8;
using namespace iv::graphics;

GraphicalBackend* backend;

void NODE_Surface2D::Init(Handle<Object> exports) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Surface2D"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("width"), FunctionTemplate::New(NODE_width)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("height"), FunctionTemplate::New(NODE_height)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("stride"), FunctionTemplate::New(NODE_stride)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("bindTexture"), FunctionTemplate::New(NODE_bindTexture)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("uploadTexture"), FunctionTemplate::New(NODE_uploadTexture)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("unbindTexture"), FunctionTemplate::New(NODE_unbindTexture)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("save"), FunctionTemplate::New(NODE_save)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());

    // Export constructor.
    exports->Set(String::NewSymbol("Surface2D"), constructor);
}

NODE_Surface2D::NODE_Surface2D(int width, int height) {
    surface = backend->createSurface2D(width, height);
}

NODE_Surface2D::~NODE_Surface2D() {
    delete surface;
}

v8::Handle<v8::Value> NODE_Surface2D::New(const v8::Arguments& args) {
    HandleScope scope;

    if(args.IsConstructCall()) {
        // Invoked as constructor: `new MyObject(...)`
        int width = args[0]->IsUndefined() ? 0 : args[0]->IntegerValue();
        int height = args[0]->IsUndefined() ? 0 : args[1]->IntegerValue();
        NODE_Surface2D* obj = new NODE_Surface2D(width, height);
        obj->Wrap(args.This());
        return args.This();
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 2;
        Local<Value> argv[argc] = { args[0], args[1] };
        return scope.Close(constructor->NewInstance(argc, argv));
    }
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_width(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    return Uint32::New(obj->surface->width());
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_height(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    return Uint32::New(obj->surface->height());
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_stride(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    return Uint32::New(obj->surface->stride());
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_bindTexture(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    obj->surface->bindTexture(args[0]->IntegerValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_uploadTexture(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    obj->surface->uploadTexture();
    return args.This();
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_unbindTexture(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    obj->surface->unbindTexture(args[0]->IntegerValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Surface2D::NODE_save(const v8::Arguments& args) {
    NODE_Surface2D* obj = ObjectWrap::Unwrap<NODE_Surface2D>(args.This());
    obj->surface->save(0);
    return args.This();
}

void NODE_GraphicalContext2D::Init(Handle<Object> exports) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("GraphicalContext2D"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("path"), FunctionTemplate::New(NODE_path)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("paint"), FunctionTemplate::New(NODE_paint)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("drawPath"), FunctionTemplate::New(NODE_drawPath)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("drawText"), FunctionTemplate::New(NODE_drawText)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("drawLine"), FunctionTemplate::New(NODE_drawLine)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("drawCircle"), FunctionTemplate::New(NODE_drawCircle)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("rotate"), FunctionTemplate::New(NODE_rotate)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("translate"), FunctionTemplate::New(NODE_translate)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("scale"), FunctionTemplate::New(NODE_scale)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("concatTransform"), FunctionTemplate::New(NODE_concatTransform)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setTransform"), FunctionTemplate::New(NODE_setTransform)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("getTransform"), FunctionTemplate::New(NODE_getTransform)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("clear"), FunctionTemplate::New(NODE_clear)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("reset"), FunctionTemplate::New(NODE_reset)->GetFunction());
    // tpl->PrototypeTemplate()->Set(
    //     String::NewSymbol("save"), FunctionTemplate::New(NODE_save)->GetFunction());
    // tpl->PrototypeTemplate()->Set(
    //     String::NewSymbol("load"), FunctionTemplate::New(NODE_load)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("push"), FunctionTemplate::New(NODE_push)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("pop"), FunctionTemplate::New(NODE_pop)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());

    // Export constructor.
    exports->Set(String::NewSymbol("GraphicalContext2D"), constructor);
}

NODE_GraphicalContext2D::NODE_GraphicalContext2D(NODE_Surface2D* surface) {
    context = backend->createGraphicalContext(surface->surface);
}

NODE_GraphicalContext2D::~NODE_GraphicalContext2D() {
    delete context;
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::New(const v8::Arguments& args) {
    HandleScope scope;

    if(args.IsConstructCall()) {
        // Invoked as constructor: `new MyObject(...)`
        NODE_Surface2D* surface = node::ObjectWrap::Unwrap<NODE_Surface2D>(args[0]->ToObject());
        NODE_GraphicalContext2D* obj = new NODE_GraphicalContext2D(surface);
        obj->Wrap(args.This());
        return args.This();
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        return scope.Close(constructor->NewInstance(argc, argv));
    }
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_path(const v8::Arguments& args) {
    const int argc = 1;
    Local<Value> argv[argc] = { args.This() };
    return NODE_Path2D::constructor->NewInstance(argc, argv);
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_paint(const v8::Arguments& args) {
    const int argc = 1;
    Local<Value> argv[argc] = { args.This() };
    return NODE_Paint2D::constructor->NewInstance(argc, argv);
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_drawPath(const v8::Arguments& args) {
    Path* path = ObjectWrap::Unwrap<NODE_Path2D>(args[0]->ToObject())->path;
    Paint* paint = ObjectWrap::Unwrap<NODE_Paint2D>(args[1]->ToObject())->paint;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->drawPath(path, paint);
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_drawText(const v8::Arguments& args) {
    String::Utf8Value str(args[0]);
    double x = args[1]->NumberValue();
    double y = args[2]->NumberValue();
    Paint* paint = ObjectWrap::Unwrap<NODE_Paint2D>(args[3]->ToObject())->paint;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->drawText(std::string(*str, *str + str.length()), x, y, paint);
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_drawLine(const v8::Arguments& args) {
    double x1 = args[0]->NumberValue();
    double y1 = args[1]->NumberValue();
    double x2 = args[2]->NumberValue();
    double y2 = args[3]->NumberValue();
    Paint* paint = ObjectWrap::Unwrap<NODE_Paint2D>(args[4]->ToObject())->paint;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->drawLine(iv::Vector3(x1, y1, 0), iv::Vector3(x2, y2, 0), paint);
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_drawCircle(const v8::Arguments& args) {
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    double radius = args[2]->NumberValue();
    Paint* paint = ObjectWrap::Unwrap<NODE_Paint2D>(args[3]->ToObject())->paint;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->drawCircle(iv::Vector3(x, y, 0), radius, paint);
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_rotate(const v8::Arguments& args) {
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->rotate(args[0]->NumberValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_translate(const v8::Arguments& args) {
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->translate(args[0]->NumberValue(), args[1]->NumberValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_scale(const v8::Arguments& args) {
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->scale(args[0]->NumberValue(), args[1]->NumberValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_concatTransform(const v8::Arguments& args) {
    iv::Matrix3 m;
    double a = args[0]->NumberValue();
    double b = args[1]->NumberValue();
    double c = args[2]->NumberValue();
    double d = args[3]->NumberValue();
    double e = args[4]->NumberValue();
    double f = args[5]->NumberValue();
    // a c e
    // b d f
    // 0 0 1
    m.a11 = a; m.a12 = c; m.a13 = e;
    m.a21 = b; m.a22 = d; m.a23 = f;
    m.a31 = 0; m.a32 = 0; m.a33 = 1;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->concatTransform(m);
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_setTransform(const v8::Arguments& args) {
    iv::Matrix3 m;
    double a = args[0]->NumberValue();
    double b = args[1]->NumberValue();
    double c = args[2]->NumberValue();
    double d = args[3]->NumberValue();
    double e = args[4]->NumberValue();
    double f = args[5]->NumberValue();
    // a c e
    // b d f
    // 0 0 1
    m.a11 = a; m.a12 = c; m.a13 = e;
    m.a21 = b; m.a22 = d; m.a23 = f;
    m.a31 = 0; m.a32 = 0; m.a33 = 1;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->setTransform(m);
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_getTransform(const v8::Arguments& args) {
    HandleScope scope;
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    iv::Matrix3 m = self->context->getTransform();
    Handle<Array> array = Array::New(6);
    array->Set(0, Number::New(m.a11));
    array->Set(1, Number::New(m.a21));
    array->Set(2, Number::New(m.a12));
    array->Set(3, Number::New(m.a22));
    array->Set(4, Number::New(m.a13));
    array->Set(5, Number::New(m.a23));
    return scope.Close(array);
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_clear(const v8::Arguments& args) {
    double r = args[0]->NumberValue() / 255.0;
    double g = args[1]->NumberValue() / 255.0;
    double b = args[2]->NumberValue() / 255.0;
    double a = args.Length() < 4 ? 1 : args[3]->NumberValue();
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->clear(iv::Color(r, g, b, a));
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_reset(const v8::Arguments& args) {
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->reset();
    return args.This();
}

// v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_save(const v8::Arguments& args) {
//     return Undefined();
// }

// v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_load(const v8::Arguments& args) {
//     return Undefined();
// }

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_push(const v8::Arguments& args) {
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->push();
    return args.This();
}

v8::Handle<v8::Value> NODE_GraphicalContext2D::NODE_pop(const v8::Arguments& args) {
    NODE_GraphicalContext2D* self = ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args.This());
    self->context->pop();
    return args.This();
}

void NODE_Path2D::Init(Handle<Object> exports) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Path2D"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("moveTo"), FunctionTemplate::New(NODE_moveTo)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("lineTo"), FunctionTemplate::New(NODE_lineTo)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("bezierCurveTo"), FunctionTemplate::New(NODE_bezierCurveTo)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("circle"), FunctionTemplate::New(NODE_circle)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("arc"), FunctionTemplate::New(NODE_arc)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());

    // Export constructor.
    exports->Set(String::NewSymbol("Path2D"), constructor);
}

NODE_Path2D::NODE_Path2D(NODE_GraphicalContext2D* context) {
    path = context->context->path();
}

NODE_Path2D::~NODE_Path2D() {
    delete path;
}

v8::Handle<v8::Value> NODE_Path2D::New(const v8::Arguments& args) {
    HandleScope scope;

    if(args.IsConstructCall()) {
        // Invoked as constructor: `new MyObject(...)`
        NODE_GraphicalContext2D* context = node::ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args[0]->ToObject());
        NODE_Path2D* obj = new NODE_Path2D(context);
        obj->Wrap(args.This());
        return args.This();
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        return scope.Close(constructor->NewInstance(argc, argv));
    }
}

v8::Handle<v8::Value> NODE_Path2D::NODE_moveTo(const v8::Arguments& args) {
    NODE_Path2D* self = ObjectWrap::Unwrap<NODE_Path2D>(args.This());
    self->path->moveTo(iv::Vector3(args[0]->NumberValue(), args[1]->NumberValue(), 0));
    return args.This();
}

v8::Handle<v8::Value> NODE_Path2D::NODE_lineTo(const v8::Arguments& args) {
    NODE_Path2D* self = ObjectWrap::Unwrap<NODE_Path2D>(args.This());
    self->path->lineTo(iv::Vector3(args[0]->NumberValue(), args[1]->NumberValue(), 0));
    return args.This();
}

v8::Handle<v8::Value> NODE_Path2D::NODE_bezierCurveTo(const v8::Arguments& args) {
    NODE_Path2D* self = ObjectWrap::Unwrap<NODE_Path2D>(args.This());
    self->path->bezierCurveTo(iv::Vector3(args[0]->NumberValue(), args[1]->NumberValue(), 0), // c1
                              iv::Vector3(args[2]->NumberValue(), args[3]->NumberValue(), 0), // c2
                              iv::Vector3(args[4]->NumberValue(), args[5]->NumberValue(), 0)); // p
    return args.This();
}

v8::Handle<v8::Value> NODE_Path2D::NODE_circle(const v8::Arguments& args) {
    NODE_Path2D* self = ObjectWrap::Unwrap<NODE_Path2D>(args.This());
    self->path->circle(iv::Vector3(args[0]->NumberValue(), args[1]->NumberValue(), 0), // center
                       args[2]->NumberValue(), // radius
                       iv::Vector3(0, 0, 1)); // normal
    return args.This();
}

v8::Handle<v8::Value> NODE_Path2D::NODE_arc(const v8::Arguments& args) {
    NODE_Path2D* self = ObjectWrap::Unwrap<NODE_Path2D>(args.This());
    self->path->arc(iv::Vector3(args[0]->NumberValue(), args[1]->NumberValue(), 0), // center
                    args[2]->NumberValue(), // radius
                    args[3]->NumberValue(), args[4]->NumberValue()); // angle1, angle2
    return args.This();
}

void NODE_Paint2D::Init(Handle<Object> exports) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Paint2D"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setMode"), FunctionTemplate::New(NODE_setMode)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setColor"), FunctionTemplate::New(NODE_setColor)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setStrokeWidth"), FunctionTemplate::New(NODE_setStrokeWidth)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setLineCap"), FunctionTemplate::New(NODE_setLineCap)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setLineJoin"), FunctionTemplate::New(NODE_setLineJoin)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setTextSize"), FunctionTemplate::New(NODE_setTextSize)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setTextAlign"), FunctionTemplate::New(NODE_setTextAlign)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setTypeface"), FunctionTemplate::New(NODE_setTypeface)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("measureText"), FunctionTemplate::New(NODE_measureText)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());

    // Export constructor.
    exports->Set(String::NewSymbol("Paint2D"), constructor);
}

NODE_Paint2D::NODE_Paint2D(NODE_GraphicalContext2D* context) {
    paint = context->context->paint();
}

NODE_Paint2D::~NODE_Paint2D() {
    delete paint;
}

v8::Handle<v8::Value> NODE_Paint2D::New(const v8::Arguments& args) {
    HandleScope scope;

    if(args.IsConstructCall()) {
        // Invoked as constructor: `new MyObject(...)`
        NODE_GraphicalContext2D* context = node::ObjectWrap::Unwrap<NODE_GraphicalContext2D>(args[0]->ToObject());
        NODE_Paint2D* obj = new NODE_Paint2D(context);
        obj->Wrap(args.This());
        return args.This();
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        return scope.Close(constructor->NewInstance(argc, argv));
    }
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setMode(const v8::Arguments& args) {
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setMode((PaintMode)args[0]->IntegerValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setColor(const v8::Arguments& args) {
    double r = args[0]->NumberValue() / 255.0;
    double g = args[1]->NumberValue() / 255.0;
    double b = args[2]->NumberValue() / 255.0;
    double a = args.Length() < 4 ? 1 : args[3]->NumberValue();
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setColor(iv::Color(r, g, b, a));
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setStrokeWidth(const v8::Arguments& args) {
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setStrokeWidth(args[0]->NumberValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setLineCap(const v8::Arguments& args) {
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setLineCap((LineCap)args[0]->IntegerValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setLineJoin(const v8::Arguments& args) {
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setLineJoin((LineJoin)args[0]->IntegerValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setTextSize(const v8::Arguments& args) {
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setTextSize(args[0]->NumberValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setTextAlign(const v8::Arguments& args) {
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    self->paint->setTextAlign((TextAlign)args[0]->IntegerValue());
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_setTypeface(const v8::Arguments& args) {
    String::Utf8Value str(args[0]);
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    FontStyle style = FontStyle::NORMAL;
    if(args.Length() >= 2) {
        style = (FontStyle)args[1]->IntegerValue();
    }
    self->paint->setTypeface(std::string(*str, *str + str.length()), style);
    return args.This();
}

v8::Handle<v8::Value> NODE_Paint2D::NODE_measureText(const v8::Arguments& args) {
    HandleScope scope;
    String::Utf8Value str(args[0]);
    NODE_Paint2D* self = ObjectWrap::Unwrap<NODE_Paint2D>(args.This());
    double width = self->paint->measureText(std::string(*str, *str + str.length()));
    return scope.Close(Number::New(width));
}


Persistent<Function> NODE_Surface2D::constructor;
Persistent<Function> NODE_GraphicalContext2D::constructor;
Persistent<Function> NODE_Path2D::constructor;
Persistent<Function> NODE_Paint2D::constructor;

void NODE_init(Handle<Object> exports) {
    backend = GraphicalBackend::CreateSkia();

    // Classes.
    NODE_Surface2D::Init(exports);
    NODE_GraphicalContext2D::Init(exports);
    NODE_Path2D::Init(exports);
    NODE_Paint2D::Init(exports);

    // Constant values.
    exports->Set(String::NewSymbol("LINECAP_BUTT"), Uint32::New((int32_t)LineCap::BUTT));
    exports->Set(String::NewSymbol("LINECAP_ROUND"), Uint32::New((int32_t)LineCap::ROUND));
    exports->Set(String::NewSymbol("LINECAP_SQUARE"), Uint32::New((int32_t)LineCap::SQUARE));

    exports->Set(String::NewSymbol("LINEJOIN_BEVEL"), Uint32::New((int32_t)LineJoin::BEVEL));
    exports->Set(String::NewSymbol("LINEJOIN_MITER"), Uint32::New((int32_t)LineJoin::MITER));
    exports->Set(String::NewSymbol("LINEJOIN_ROUND"), Uint32::New((int32_t)LineJoin::ROUND));

    exports->Set(String::NewSymbol("FONTSTYLE_NORMAL"), Uint32::New((int32_t)FontStyle::NORMAL));
    exports->Set(String::NewSymbol("FONTSTYLE_BOLD"), Uint32::New((int32_t)FontStyle::BOLD));
    exports->Set(String::NewSymbol("FONTSTYLE_ITALIC"), Uint32::New((int32_t)FontStyle::ITALIC));
    exports->Set(String::NewSymbol("FONTSTYLE_BOLDITALIC"), Uint32::New((int32_t)FontStyle::BOLDITALIC));

    exports->Set(String::NewSymbol("TEXTALIGN_LEFT"), Uint32::New((int32_t)TextAlign::LEFT));
    exports->Set(String::NewSymbol("TEXTALIGN_CENTER"), Uint32::New((int32_t)TextAlign::CENTER));
    exports->Set(String::NewSymbol("TEXTALIGN_RIGHT"), Uint32::New((int32_t)TextAlign::RIGHT));

    exports->Set(String::NewSymbol("PAINTMODE_STROKE"), Uint32::New((int32_t)PaintMode::STROKE));
    exports->Set(String::NewSymbol("PAINTMODE_FILL"), Uint32::New((int32_t)PaintMode::FILL));
    exports->Set(String::NewSymbol("PAINTMODE_STROKEFILL"), Uint32::New((int32_t)PaintMode::STROKEFILL));
}

NODE_MODULE(node_graphics, NODE_init)
