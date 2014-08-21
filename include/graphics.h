#ifndef IV_GRAPHICS_H
#define IV_GRAPHICS_H

#include "common.h"
#include "stream.h"
#include "math/math.hpp"

IVNS_BEGIN

namespace graphics {

    // Graphics API based on Skia's API.

    class Path;
    class Paint;

    enum class LineCap {
        BUTT, ROUND, SQUARE
    };

    enum class LineJoin {
        BEVEL, MITER, ROUND
    };

    enum class FontStyle {
        NORMAL, BOLD, ITALIC, BOLDITALIC
    };

    enum class TextAlign {
        LEFT, CENTER, RIGHT
    };

    enum class PaintMode {
        STROKE, FILL, STROKEFILL
    };

    // Abstract class for 2D graphical contexts.
    class GraphicalContext {
    public:

        // The initial origin is defined at the center of the canvas.
        // X: left to right, Y: bottom to top.

        struct State {
            Matrix3 transform;
        };

        // Begin a path, delete the path after use.
        virtual Path* path() = 0;
        virtual Paint* paint() = 0;

        // Draw a path.
        virtual void drawPath(Path* path, Paint* paint) = 0;
        // Draw text.
        virtual void drawText(const char* text, double x, double y, Paint* paint) = 0;
        void drawText(const std::string& text, double x, double y, Paint* paint) {
            drawText(text.c_str(), x, y, paint);
        }
        // Draw line.
        virtual void drawLine(Vector3 p1, Vector3 p2, Paint* paint) = 0;
        // Draw line.
        virtual void drawCircle(Vector3 center, double radius, Paint* paint) = 0;

        virtual void rotate(double radius) = 0;
        virtual void translate(double tx, double ty) = 0;
        virtual void scale(double tx, double ty) = 0;
        virtual void concatTransform(const Matrix3& matrix) = 0;
        virtual void setTransform(const Matrix3& matrix) = 0;
        virtual Matrix3 getTransform() const = 0;

        // Fill the entire canvas with color.
        virtual void clear(const Color& color) = 0;

        // Reset the graphical state.
        virtual void reset() = 0;

        // Save/load the current graphical state.
        virtual State save() const = 0;
        virtual void load(const State& state) = 0;

        // Push and pop the graphical state.
        virtual void push() = 0;
        virtual void pop() = 0;

        virtual ~GraphicalContext() { }

    };

    class GraphicalContext3D {
    };

    // This can be used for both 2D and 3D contexts,
    // for 2D drawing, only x, y components will be considered.
    class Path {
    public:

        // Like most graphical contexts, we can construct a path with commands:
        // Move the pen position to p.
        virtual void moveTo(const Vector3& p) = 0;
        // Add a line from the pen position to p, and move the pen to p.
        virtual void lineTo(const Vector3& p) = 0;
        // Add a bezier curve, with four points: the pen, c1, c2, p, and move the pen to p.
        virtual void bezierCurveTo(const Vector3& c1, const Vector3& c2, const Vector3& p) = 0;
        // Draw circle centered at center, with normal, and radius.
        virtual void circle(const Vector3& center, double radius, const Vector3& normal) = 0;

        // Draw 2D arc centered at center, radius, and angle1 to angle2.
        virtual void arc(const Vector3& center, double radius, double angle1, double angle2) = 0;

        virtual ~Path() { }

    };

    class Paint {
    public:

        virtual void setMode(const PaintMode& mode) = 0;

        // Set stroke/fill styles.
        virtual void setColor(const Color& color) = 0;
        virtual void setStrokeWidth(double value) = 0;
        virtual void setLineCap(LineCap value) = 0;
        virtual void setLineJoin(LineJoin value) = 0;

        // Font styles.
        virtual void setTextSize(double value) = 0;
        virtual void setTextAlign(TextAlign align) = 0;
        virtual void setTypeface(const char* name, FontStyle style = FontStyle::NORMAL) = 0;
        void setTypeface(const std::string& name, FontStyle style = FontStyle::NORMAL) {
            setTypeface(name.c_str(), style);
        }

        // Measure text width.
        virtual double measureText(const char* text) = 0;
        double measureText(const std::string& text) { return measureText(text.c_str()); };

        virtual ~Paint() { }
    };

    // 2D surface.
    class Surface2D {
    public:

        // Width, height, stride.
        virtual int width() const = 0;
        virtual int height() const = 0;
        virtual int stride() const = 0;

        // Get a pointer to the pixels.
        // RGBA, unsigned byte format.
        virtual unsigned char* pixels() = 0;

        // Create a OpenGL texture for the surface.
        virtual void bindTexture(unsigned int unit) = 0;
        virtual void uploadTexture() = 0;
        virtual void unbindTexture(unsigned int unit) = 0;

        virtual void save(ByteStream* stream) = 0;

        virtual ~Surface2D() { }

    };

    class GraphicalBackend {
    public:

        // Create new 2D surface.
        virtual Surface2D* createSurface2D(int width, int height) = 0;
        // Render to PDF!
        virtual Surface2D* createPDFSurface2D(int width, int height) = 0;

        // Create 2D graphical context for a surface.
        virtual GraphicalContext* createGraphicalContext(Surface2D* surface) = 0;

        virtual ~GraphicalBackend() { }

        // Create different backends.
        static GraphicalBackend* CreateSkia();
        static GraphicalBackend* CreateCairo();

    };
}

IVNS_END

#endif
