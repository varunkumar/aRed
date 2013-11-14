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

#ifndef _QCAR_QUAD_H_
#define _QCAR_QUAD_H_


#define NUM_QUAD_VERTEX 4
#define NUM_QUAD_INDEX 6


/*static const float quadVertices[NUM_QUAD_VERTEX * 3] =
{
   -1.00f,  -1.00f,  -1.00f,
    1.00f,  -1.00f,  -1.00f,
    1.00f,   1.00f,  -1.00f,
   -1.00f,   1.00f,  -1.00f,
};*/

static const float quadTexCoords[NUM_QUAD_VERTEX * 2] =
{
    0, 0,
    1, 0,
    1, 1,
    0, 1,
};

static const float quadNormals[NUM_QUAD_VERTEX * 3] =
{
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,

};

/*static const unsigned short quadIndices[NUM_QUAD_INDEX] =
{
     0,  1,  2,  0,  2,  3,
};*/


#endif // _QC_AR_QUAD_H_
