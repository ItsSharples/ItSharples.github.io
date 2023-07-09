#pragma once
namespace MyMaths
{
#if defined __linux__ || defined __APPLE__ 
    // "Compiled for Linux
#else 
    // Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793 
//#define INFINITY 1e8 
#endif 
    constexpr auto MAX_RAY_DEPTH = 5;
#include <float.h>
    constexpr float F_Epsilon = FLT_EPSILON; // Smallest
    constexpr float F_Infinity = FLT_MAX; // Biggest

    float mix(const float& a, const float& b, const float& mix);
};
