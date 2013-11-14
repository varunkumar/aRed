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
    VideoPlaybackShaders.h

@brief
    Defines OpenGL shaders as char* strings.
    These are used to render the video frames from the SurfaceTexture

==============================================================================*/

#ifndef _QCAR_VIDEOPLAYBACK_SHADERS_H_
#define _QCAR_VIDEOPLAYBACK_SHADERS_H_

static const char* videoPlaybackVertexShader =
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

/*
 *
 * IMPORTANT:
 *
 * The SurfaceTexture functionality from ICS provides the video
 * frames from the movie in an unconventional format. So we cant
 * use Texture2D but we need to use the ExternalOES extension.
 *
 * Two things that are important in the shader below. The first is
 * the extension declaration (first line). The second is the type
 * of the texSamplerOES uniform.
 *
 */

static const char videoPlaybackFragmentShader[] =
    "#extension GL_OES_EGL_image_external : require \n"
    "precision mediump float; \n"
    "varying vec2 texCoord; \n"
    "uniform samplerExternalOES texSamplerOES; \n"
    "void main() \n"
    "{ \n"
    "   gl_FragColor = texture2D(texSamplerOES, texCoord); \n"
    "} \n";

#endif // _QCAR_VIDEOPLAYBACK_SHADERS_H_
