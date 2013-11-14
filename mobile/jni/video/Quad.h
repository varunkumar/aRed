/*==============================================================================
Copyright (c) 2012-2013 QUALCOMM Austria Research Center GmbH.
All Rights Reserved.

This  Vuforia(TM) sample application in source code form ("Sample Code") for the
Vuforia Software Development Kit and/or Vuforia Extension for Unity
(collectively, the "Vuforia SDK") may in all cases only be used in conjunction
with use of the Vuforia SDK, and is subject in all respects to all of the terms
and conditions of the Vuforia SDK License Agreement, which may be found at
https://developer.vuforia.com/legal/license.

By retaining or using the Sample Code in any manner, you confirm your agreement
to all the terms and conditions of the Vuforia SDK License Agreement.  If you do
not agree to all the terms and conditions of the Vuforia SDK License Agreement,
then you may not retain or use any of the Sample Code in any manner.


@file
    Quad.h

@brief
    Geometry for a simple quad used in the samples.

==============================================================================*/

#ifndef _QCAR_QUAD_H_
#define _QCAR_QUAD_H_


#define NUM_QUAD_VERTEX 4
#define NUM_QUAD_INDEX 6


static const float quadVertices[NUM_QUAD_VERTEX * 3] =
{
   -1.00f,  -1.00f,  -1.00f,
    1.00f,  -1.00f,  -1.00f,
    1.00f,   1.00f,  -1.00f,
   -1.00f,   1.00f,  -1.00f,
};

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

static const unsigned short quadIndices[NUM_QUAD_INDEX] =
{
     0,  1,  2,  0,  2,  3,
};


#endif // _QC_AR_QUAD_H_
