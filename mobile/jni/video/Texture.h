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
    Texture.h

@brief
    A utility class for textures used in the samples.

==============================================================================*/
#ifndef _QCAR_TEXTURE_H_
#define _QCAR_TEXTURE_H_

// Include files
#include <jni.h>

// Forward declarations

/// A utility class for textures.
class Texture
{
public:

    /// Constructor
    Texture();

    /// Destructor.
    ~Texture();

    /// Returns the width of the texture.
    unsigned int getWidth() const;

    /// Returns the height of the texture.
    unsigned int getHeight() const;

    /// Create a texture from a jni object:
    static Texture* create(JNIEnv* env, jobject textureObject);
 
    /// The width of the texture.
    unsigned int mWidth;

    /// The height of the texture.
    unsigned int mHeight;

    /// The number of channels of the texture.
    unsigned int mChannelCount;

    /// Whether the texture was succesfully loaded
    bool mSuccess;

    /// The pointer to the raw texture data.
    unsigned char* mData;

    /// The ID of the texture
    unsigned int mTextureID;
    
private: 

    /// Hidden copy constructor
    Texture(const Texture &);      
           
    /// Hidden assignment operator
    Texture& operator= (const Texture &); 
    
};


#endif //_QCAR_TEXTURE_H_
