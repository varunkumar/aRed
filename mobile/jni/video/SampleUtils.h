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
    SampleUtils.h

@brief
    A utility class.

==============================================================================*/


#ifndef _QCAR_SAMPLEUTILS_H_
#define _QCAR_SAMPLEUTILS_H_

// Includes:
#include <stdio.h>
#include <android/log.h>

// Utility for logging:
#define LOG_TAG    "QCAR"
#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/// A utility class used by the QCAR SDK samples.
class SampleUtils
{
public:

    /// Prints a 4x4 matrix.
    static void printMatrix(const float* matrix);

    /// Prints GL error information.
    static void checkGlError(const char* operation);

    /// Set the rotation components of this 4x4 matrix.
    static void setRotationMatrix(float angle, float x, float y, float z,
        float *nMatrix);

    /// Set the translation components of this 4x4 matrix.
    static void translatePoseMatrix(float x, float y, float z,
        float* nMatrix = NULL);

    /// Applies a rotation.
    static void rotatePoseMatrix(float angle, float x, float y, float z,
        float* nMatrix = NULL);

    /// Applies a scaling transformation.
    static void scalePoseMatrix(float x, float y, float z,
        float* nMatrix = NULL);

    /// Multiplies the two matrices A and B and writes the result to C.
    static void multiplyMatrix(float *matrixA, float *matrixB,
        float *matrixC);

    /// Initialize a shader.
    static unsigned int initShader(unsigned int shaderType,
        const char* source);

    /// Create a shader program.
    static unsigned int createProgramFromBuffer(const char* vertexShaderBuffer,
        const char* fragmentShaderBuffer);
};

#endif // _QCAR_SAMPLEUTILS_H_
