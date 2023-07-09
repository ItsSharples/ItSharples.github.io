#include "MyMaths.h"

float MyMaths::mix(const float& a, const float& b, const float& mix)
{
    return b * mix + a * (1 - mix);
}
