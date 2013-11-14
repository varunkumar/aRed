#ifndef _QCAR_QUAD_OBJECT_H_
#define _QCAR_QUAD_OBJECT_H_

#define NUM_QUAD_OBJECT_INDICES 8

static const float quadVertices[] =
{
    -0.5, -0.5, 0.0, 0.5, -0.5, 0.0, 0.5, 0.5, 0.0, -0.5, 0.5, 0.0,
};

static const unsigned short quadIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0
};

#endif // _QCAR_QUAD_OBJECT_H_
