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
    KeyframeShaders.h

@brief
    Defines OpenGL shaders as char* strings.
    These are used to render the keyframe and the icons

==============================================================================*/

#ifndef _QCAR_KEYFRAME_SHADERS_H_
#define _QCAR_KEYFRAME_SHADERS_H_

static const char* keyframeVertexShader =
    "attribute vec4 vertexPosition; \n"
    "attribute vec4 vertexNormal; \n"
    "attribute vec2 vertexTexCoord; \n"
    "varying vec2 texCoord; \n"
    "varying vec4 normal; \n"
    "uniform mat4 modelViewProjectionMatrix; \n"
    "void main() \n"
    "{ \n"
    "   gl_Position = modelViewProjectionMatrix * vertexPosition; \n"
    "   normal = vertexNormal; \n"
    "   texCoord = vertexTexCoord; \n"
    "} \n";

static const char keyframeFragmentShader[] =
    "precision mediump float; \n"
    "varying vec2 texCoord; \n"
    "uniform sampler2D texSampler2D; \n"
    "void main() \n"
    "{ \n"
    "   gl_FragColor = texture2D(texSampler2D, texCoord); \n"
    "} \n";

#endif // _QCAR_KEYFRAME_SHADERS_H_
