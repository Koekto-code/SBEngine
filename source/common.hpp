// 20 june 2022
// SB Engine v0.4.0-alpha

#ifndef COMMON_HPP
#define COMMON_HPP

#define GLFW_DLL
#define GLFW_INCLUDE_NONE

#undef _DEBUG
#define _DEBUG 1

#ifndef NAMESPACE_BEGIN
    #define NAMESPACE_BEGIN(name) namespace name {
#endif

#ifndef NAMESPACE_END
    #define NAMESPACE_END(name) }
#endif

#include <cstdint>
#include <cstddef>

#endif