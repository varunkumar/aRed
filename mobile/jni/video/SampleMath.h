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
    SampleMath.h

@brief
    A utility class.

==============================================================================*/


#ifndef _QCAR_SAMPLEMATH_H_
#define _QCAR_SAMPLEMATH_H_

// Includes:
#include <QCAR/Tool.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Renderer.h>

/// A utility class used by the QCAR SDK samples.
class SampleMath
{
public:

    static QCAR::Vec2F Vec2FSub(QCAR::Vec2F v1, QCAR::Vec2F v2);

    static float Vec2FDist(QCAR::Vec2F v1, QCAR::Vec2F v2);

    static QCAR::Vec3F Vec3FAdd(QCAR::Vec3F v1, QCAR::Vec3F v2);

    static QCAR::Vec3F Vec3FSub(QCAR::Vec3F v1, QCAR::Vec3F v2);

    static QCAR::Vec3F Vec3FScale(QCAR::Vec3F v, float s);

    static float Vec3FDot(QCAR::Vec3F v1, QCAR::Vec3F v2);

    static QCAR::Vec3F Vec3FCross(QCAR::Vec3F v1, QCAR::Vec3F v2);

    static QCAR::Vec3F Vec3FNormalize(QCAR::Vec3F v);

    static QCAR::Vec3F Vec3FTransform(QCAR::Vec3F& v, QCAR::Matrix44F& m);

    static QCAR::Vec3F Vec3FTransformNormal(QCAR::Vec3F& v, QCAR::Matrix44F& m);

    static QCAR::Vec4F Vec4FTransform(QCAR::Vec4F& v, QCAR::Matrix44F& m);

    static QCAR::Vec4F Vec4FDiv(QCAR::Vec4F v1, float s);

    static QCAR::Matrix44F Matrix44FIdentity();

    static QCAR::Matrix44F Matrix44FTranspose(QCAR::Matrix44F m);

    static float Matrix44FDeterminate(QCAR::Matrix44F& m);

    static QCAR::Matrix44F Matrix44FInverse(QCAR::Matrix44F& m);

    static bool linePlaneIntersection(QCAR::Vec3F lineStart, QCAR::Vec3F lineEnd,
                      QCAR::Vec3F pointOnPlane, QCAR::Vec3F planeNormal,
                      QCAR::Vec3F &intersection);

    static void projectScreenPointToPlane(QCAR::Matrix44F inverseProjMatrix, QCAR::Matrix44F modelViewMatrix,
                                          float screenWidth, float screenHeight,
                                          QCAR::Vec2F point, QCAR::Vec3F planeCenter, QCAR::Vec3F planeNormal,
                                          QCAR::Vec3F &intersection, QCAR::Vec3F &lineStart, QCAR::Vec3F &lineEnd);
};

#endif // _QCAR_SAMPLEMATH_H_
