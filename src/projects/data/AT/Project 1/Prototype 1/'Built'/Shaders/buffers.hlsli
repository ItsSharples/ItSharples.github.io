cbuffer MatrixBuffer
{
    //matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CoordBuffer
{
    float4 coord_pos;
    //float4 coord_rot;
};