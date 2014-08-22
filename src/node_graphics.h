#include <node.h>
#include <v8.h>

#include <graphics.h>


class NODE_Surface2D : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports);

    iv::graphics::Surface2D* surface;

private:
    explicit NODE_Surface2D(int width, int height);
    ~NODE_Surface2D();

    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_width(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_height(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_bindTexture(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_uploadTexture(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_unbindTexture(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_save(const v8::Arguments& args);

    static v8::Persistent<v8::Function> constructor;
};

class NODE_GraphicalContext2D : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports);

    iv::graphics::GraphicalContext* context;

private:
    explicit NODE_GraphicalContext2D(NODE_Surface2D* surface);
    ~NODE_GraphicalContext2D();

    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_path(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_paint(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_drawPath(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_drawText(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_drawLine(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_drawCircle(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_rotate(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_translate(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_scale(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_concatTransform(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setTransform(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_getTransform(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_clear(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_reset(const v8::Arguments& args);

    // static v8::Handle<v8::Value> NODE_getState(const v8::Arguments& args);
    // static v8::Handle<v8::Value> NODE_setState(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_save(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_restore(const v8::Arguments& args);

    static v8::Persistent<v8::Function> constructor;
};

class NODE_Path2D : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports);

    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);

    iv::graphics::Path* path;

    friend class NODE_GraphicalContext2D;

private:
    explicit NODE_Path2D(NODE_GraphicalContext2D* context);
    ~NODE_Path2D();

    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_moveTo(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_lineTo(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_bezierCurveTo(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_circle(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_arc(const v8::Arguments& args);

    static v8::Persistent<v8::Function> constructor;
};

class NODE_Paint2D : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports);

    static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);

    iv::graphics::Paint* paint;

    friend class NODE_GraphicalContext2D;

private:
    explicit NODE_Paint2D(NODE_GraphicalContext2D* context);
    ~NODE_Paint2D();

    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_setMode(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setColor(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setStrokeWidth(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setLineCap(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setLineJoin(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_setTextSize(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setTextAlign(const v8::Arguments& args);
    static v8::Handle<v8::Value> NODE_setTypeface(const v8::Arguments& args);

    static v8::Handle<v8::Value> NODE_measureText(const v8::Arguments& args);

    static v8::Persistent<v8::Function> constructor;
};
