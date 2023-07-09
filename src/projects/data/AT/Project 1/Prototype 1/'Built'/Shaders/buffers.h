#pragma once
#include "../Maths/Maths.h"
using namespace DXL::Maths;
struct MatrixBuffer
{
    //Matrix4x4 worldMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
};

struct CoordBuffer
{
    Vector coord_pos;
    //Vector coord_rot;
};