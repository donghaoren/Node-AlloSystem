#ifndef IV_MATH_COLOR_H
#define IV_MATH_COLOR_H

#include "vector.hpp"

IVNS_BEGIN

struct Color {
    double r, g, b, a;
    Color() = default;
    Color(double r_, double g_, double b_, double a_ = 1) : r(r_), g(g_), b(b_), a(a_) { }
};

IVNS_END

#endif
