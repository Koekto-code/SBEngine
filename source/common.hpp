// 24 june 2022
// SB Engine v0.1.0-alpha

#ifndef COMMON_HPP
#define COMMON_HPP

#define GLFW_DLL
#define GLFW_INCLUDE_NONE

#if (not NDEBUG)
    #define _DEBUG 1
#else
    #undef _DEBUG
#endif

#ifndef NAMESPACE_BEGIN
    #define NAMESPACE_BEGIN(name) namespace name {
#endif

#ifndef NAMESPACE_END
    #define NAMESPACE_END(name) }
#endif

#include <cstdint>
#include <cstddef>

#endif
