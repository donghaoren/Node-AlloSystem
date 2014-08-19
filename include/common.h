#ifndef IV_COMMON_H
#define IV_COMMON_H

#include <string>
#include <memory>
#include <exception>
#include <stdexcept>

#define IVNS_BEGIN namespace iv {
#define IVNS_END }

IVNS_BEGIN

class not_implemented_yet : public std::exception { };
class not_supported : public std::exception { };

IVNS_END

#endif
