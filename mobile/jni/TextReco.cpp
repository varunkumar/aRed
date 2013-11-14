#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef USE_OPENGL_ES_1_1
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/TrackableResult.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/Word.h>
#include <QCAR/WordResult.h>
#include <QCAR/TextTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>
#include <QCAR/Rectangle.h>



#include "SampleUtils.h"
#include "LineShaders.h"
#include "Quad.h"

#ifdef __cplusplus
extern "C"
{
#endif

unsigned int lineShaderProgramID = 0;
GLint vertexHandle               = 0;
GLint mvpMatrixHandle            = 0;
GLint lineOpacityHandle          = 0;
GLint lineColorHandle            = 0;

// Screen dimensions:
unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;


int viewportPosition_x          = 0;
int viewportPosition_y          = 0;
int viewportSize_x              = 0;
int viewportSize_y              = 0;

//pixels (screen coordinates)
int ROICenterX = 0;
int ROICenterY = 0;
int ROIWidth = 0;
int ROIHeight = 0;

float ROIOrthoProjMatrix[16];

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode   = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

static float ROIVertices[] =
{
    -0.5, -0.5, 0.0,    0.5, -0.5, 0.0,    0.5, 0.5, 0.0,    -0.5, 0.5, 0.0
};

static unsigned short ROIIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0
};

#define MAX_WORD_LENGTH 255
#define MAX_NB_WORDS 132

#define TEXTBOX_PADDING 0.0f

struct WordDesc
{
    char text[MAX_WORD_LENGTH];
    int Ax,Ay; // Upper left corner
    int Bx,By; // Lower right corner
};

// structure to hold the words discovered
static WordDesc WordsFound[MAX_NB_WORDS];
static int NbWordsFound = 0;


bool unicodeToAscii(const QCAR::Word& word, char*& asciiString);

// here 0,0 is the upper left corner
// X grows from left to right
// Y grows from top to bottom
int wordDescCompare(const void * o1, const void * o2)
{
    const WordDesc * w1 = (const WordDesc *)o1;
    const WordDesc * w2 = (const WordDesc *)o2;
    int ret = 0;
    int sameLine = 0;

    // we check first if both words are on the same line
    // both words are said to be on the same line if the
    // mid point (on Y axis) of the first point
    // is between the values of the second point
    int mid1Y = (w1->Ay + w1->By) / 2;

    if ((mid1Y < w2->By) && (mid1Y > w2->Ay))
    {
        // words are on the same line
        ret = w1->Ax - w2->Ax;
        sameLine = 1;
    }
    else
    {
        // words on different line
        ret = w1->Ay - w2->Ay;
    }

    return ret;
}

// Object to receive update callbacks from QCAR SDK
class TextReco_UpdateCallback : public QCAR::UpdateCallback
{   
    virtual void QCAR_onUpdate(QCAR::State& /*state*/)
    {
    }
};

TextReco_UpdateCallback updateCallback;

JNIEXPORT int JNICALL
Java_com_codered_ared_TextReco_getOpenGlEsVersionNative(JNIEnv *, jobject)
{
    // Only OpenGL 2
    return 2;
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
    isActivityInPortraitMode = isPortrait;
}


JNIEXPORT int JNICALL
Java_com_codered_ared_TextReco_initTracker(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_initTracker");

    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();

    // Init text tracker
    QCAR::Tracker* textTracker = trackerManager.initTracker(QCAR::Tracker::TEXT_TRACKER);
    if (textTracker == NULL)
    {
        LOG("Failed to initialize TextTracker.");
        return 0;
    }
    LOG("Successfully initialized TextTracker.");
    return 1;

}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_deinitTracker(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_deinitTracker");

    // Deinit the text tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    trackerManager.deinitTracker(QCAR::Tracker::TEXT_TRACKER);
}


JNIEXPORT int JNICALL
Java_com_codered_ared_TextReco_loadTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_loadTrackerData");
    

    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();

    QCAR::Tracker* textTracker = trackerManager.getTracker(QCAR::Tracker::TEXT_TRACKER);
    QCAR::TextTracker* tt = reinterpret_cast<QCAR::TextTracker*>(textTracker);
    tt->getWordList()->loadWordList("Vuforia-English-word.vwl", QCAR::WordList::STORAGE_APPRESOURCE);

    LOG("Successfully loaded default word list");

    return 1;
}


JNIEXPORT int JNICALL
Java_com_codered_ared_TextReco_destroyTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_destroyTrackerData");
    return 1;
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_onQCARInitializedNative(JNIEnv *, jobject)
{
    // Register the update callback where we handle the data set swap:
    QCAR::registerCallback(&updateCallback);

    // Comment in to enable tracking of up to 2 targets simultaneously and
    // split the work over multiple frames:
    // QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_setROI(JNIEnv *, jobject, jfloat center_x, jfloat center_y, jfloat width, jfloat height)
{
    ROICenterX = (float)center_x;
    ROICenterY = (float)center_y;
    ROIWidth = (float)width;
    ROIHeight = (float)height;
}


void
setOrthoMatrix(float nLeft, float nRight, float nBottom, float nTop,
    float nNear, float nFar, float *nProjMatrix)
{
    int i;
    for (i = 0; i < 16; i++)
        nProjMatrix[i] = 0.0f;

    nProjMatrix[0] = 2.0f / (nRight - nLeft);
    nProjMatrix[5] = 2.0f / (nTop - nBottom);
    nProjMatrix[10] = 2.0f / (nNear - nFar);
    nProjMatrix[12] = -(nRight + nLeft) / (nRight - nLeft);
    nProjMatrix[13] = -(nTop + nBottom) / (nTop - nBottom);
    nProjMatrix[14] = (nFar + nNear) / (nFar - nNear);
    nProjMatrix[15] = 1.0f;
}

void drawRegionOfInterest(float center_x, float center_y, float width, float height)
{
    // assumption is that center_x, center_y, width and height are given here in screen coordinates (screen pixels)

    setOrthoMatrix(
            0.0f, (float)viewportSize_x,
            (float)viewportSize_y, 0.0f,
            -1.0f, 1.0f,
            ROIOrthoProjMatrix);

    // compute coordinates
    float minX = center_x - width / 2;
    float maxX = center_x + width / 2;
    float minY = center_y - height / 2;
    float maxY = center_y + height / 2;

    //Update vertex coordinates of ROI rectangle
    ROIVertices[0] = minX - viewportPosition_x;
    ROIVertices[1] = minY - viewportPosition_y;
    ROIVertices[2] = 0;

    ROIVertices[3] = maxX - viewportPosition_x;
    ROIVertices[4] = minY - viewportPosition_y;
    ROIVertices[5] = 0;

    ROIVertices[6] = maxX - viewportPosition_x;
    ROIVertices[7] = maxY - viewportPosition_y;
    ROIVertices[8] = 0;

    ROIVertices[9] = minX - viewportPosition_x;
    ROIVertices[10] = maxY - viewportPosition_y;
    ROIVertices[11] = 0;

    glUseProgram(lineShaderProgramID);
    glLineWidth(3.0f);

    glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, ROIVertices);
    glEnableVertexAttribArray(vertexHandle);

    glUniform1f(lineOpacityHandle, 1.0f); // 0.35f);
    glUniform3f(lineColorHandle, 0.0f, 0.0f, 0.0f);//R,G,B
    glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE, ROIOrthoProjMatrix);

    // Then, we issue the render call
    glDrawElements(GL_LINES, NUM_QUAD_OBJECT_INDICES, GL_UNSIGNED_SHORT,
                           (const GLvoid*) &ROIIndices[0]);

    // Disable the vertex array handle
    glDisableVertexAttribArray(vertexHandle);

    // Restore default line width
    glLineWidth(1.0f);

    // Unbind shader program
    glUseProgram(0);
}

JNIEXPORT void JNICALL
Java_com_codered_ared_TextRecoRenderer_renderFrame(JNIEnv * env, jobject obj)
{
    //LOG("JJava_com_codered_ared_TextRecoRenderer_renderFrame");

    // Clear color and depth buffer 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Get the state from QCAR and mark the beginning of a rendering section
    QCAR::State state = QCAR::Renderer::getInstance().begin();
    
    // Explicitly render the Video Background
    QCAR::Renderer::getInstance().drawVideoBackground();

    glEnable(GL_DEPTH_TEST);

    // We need Front Face, CW for the back camera and Front Face CCW for the front camera...
    // or more accuratly, we need CW for 0 and 2 reflections and CCW for 1 reflection
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    if(QCAR::Renderer::getInstance().getVideoBackgroundConfig().mReflection == QCAR::VIDEO_BACKGROUND_REFLECTION_ON)
    {
        glFrontFace(GL_CCW);  //Front camera
    }
    else
    {
        glFrontFace(GL_CW);   //Back camera
    }

    // Enable blending to support transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    jclass rendererJavaClass = env->GetObjectClass(obj);

    env->CallVoidMethod(obj, env->GetMethodID(rendererJavaClass, "wordsStartLoop", "()V"));

    NbWordsFound = 0;

    // Did we find any trackables this frame?
    for(int tIdx = 0; tIdx < state.getNumTrackableResults(); tIdx++)
    {
        // Get the trackable:
        const QCAR::TrackableResult* result = state.getTrackableResult(tIdx);
        const QCAR::Trackable& trackable = result->getTrackable();

        QCAR::Matrix44F modelViewMatrix =
            QCAR::Tool::convertPose2GLMatrix(result->getPose());        

        QCAR::Vec2F wordBoxSize(0, 0);

        if (result->getType() == QCAR::TrackableResult::WORD_RESULT)
        {
            const QCAR::WordResult* wordResult = (const QCAR::WordResult*) result;
            // Get the word
            const QCAR::Word& word = wordResult->getTrackable();
            const QCAR::Obb2D& obb = wordResult->getObb();
            wordBoxSize = word.getSize();

            if (word.getStringU())
            {
                // in portrait, the obb coordinate is based on
                // a 0,0 position being in the upper right corner
                // with :
                // X growing from top to bottom and
                // Y growing from right to left
                //
                // we convert those coordinates to be more natural
                // with our application:
                // - 0,0 is the upper left corner
                // - X grows from left to right
                // - Y grows from top to bottom
                float wordx = - obb.getCenter().data[1];
                float wordy = obb.getCenter().data[0];

                // For debugging purposes convert the string to 7bit ASCII
                // (if possible) and log it.
                char* stringA = 0;
                if (unicodeToAscii(word, stringA))
                {
                    // we store the word
                    if (NbWordsFound < MAX_NB_WORDS)
                    {
                        struct WordDesc * word =  & WordsFound[NbWordsFound];
                        NbWordsFound++;
                        strncpy(word->text, stringA, MAX_WORD_LENGTH - 1);
                        word->text[MAX_WORD_LENGTH - 1] = '\0';
                        word->Ax = wordx - (int)(wordBoxSize.data[0] / 2);
                        word->Ay = wordy - (int)(wordBoxSize.data[1] / 2);
                        word->Bx = wordx + (int)(wordBoxSize.data[0] / 2);
                        word->By = wordy + (int)(wordBoxSize.data[1] / 2);
                    }

                    delete[] stringA;
                }
            }
        }
        else
        {
            LOG("Unexpected detection:%d", result->getType());
            continue;
        }

        QCAR::Matrix44F modelViewProjection;

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, 0.0f,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(wordBoxSize.data[0] + TEXTBOX_PADDING, wordBoxSize.data[1] + TEXTBOX_PADDING, 1.0f,
                                     &modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                    &modelViewMatrix.data[0] ,
                                    &modelViewProjection.data[0]);

        glUseProgram(lineShaderProgramID);
        glLineWidth(3.0f);

        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) &quadVertices[0]);
        
        glEnableVertexAttribArray(vertexHandle);
        
        glUniform1f(lineOpacityHandle, 1.0f);
        // FF7200
        glUniform3f(lineColorHandle, 1.0f, 0.447f, 0.0f);
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );

        glDrawElements(GL_LINES, NUM_QUAD_OBJECT_INDICES, GL_UNSIGNED_SHORT,
                       (const GLvoid*) &quadIndices[0]);

        // Disable the vertex array handle
        glDisableVertexAttribArray(vertexHandle);

        // Restore default line width
        glLineWidth(1.0f);

        // Unbind shader program
        glUseProgram(0);
    }

    if (NbWordsFound > 0)
    {
        jmethodID method = env->GetMethodID(rendererJavaClass, "addWord", "(Ljava/lang/String;)V");

        // we order the words per line and left to right
        qsort(& WordsFound[0], NbWordsFound, sizeof(struct WordDesc), wordDescCompare);
        for(int i = 0 ; i < NbWordsFound ; i++)
        {
            struct WordDesc * word =  & WordsFound[i];
            jstring js = env->NewStringUTF(word->text);
            env->CallVoidMethod(obj, method, js);
        }
    }

    env->CallVoidMethod(obj, env->GetMethodID(rendererJavaClass, "wordsEndLoop", "()V"));


    SampleUtils::checkGlError("TextReco renderFrame - words post-render");

    glDisable(GL_DEPTH_TEST);

    drawRegionOfInterest(ROICenterX, ROICenterY, ROIWidth, ROIHeight);

    // Disable blending (restore default state)
    glDisable(GL_BLEND);

    SampleUtils::checkGlError("TextReco renderFrame - post-drawROI");

    QCAR::Renderer::getInstance().end();
}


void
configureVideoBackground()
{
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.
                                getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;
    
    if (isActivityInPortraitMode)
    {
        config.mSize.data[0] = videoMode.mHeight
                                * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;

        if(config.mSize.data[0] < screenWidth)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenWidth;
            config.mSize.data[1] = screenWidth * 
                              (videoMode.mWidth / (float)videoMode.mHeight);
        }
    }
    else
    {
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
                            * (screenWidth / (float)videoMode.mWidth);

        if(config.mSize.data[1] < screenHeight)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenHeight
                                * (videoMode.mWidth / (float)videoMode.mHeight);
            config.mSize.data[1] = screenHeight;
        }

    }

    QCAR::Vec2I loupeCenter(0, 0);
    QCAR::Vec2I loupeSize(0, 0);

    // conversion to camera coordinates
    SampleUtils::screenCoordToCameraCoord(ROICenterX, ROICenterY, ROIWidth, ROIHeight,
            screenWidth, screenHeight, videoMode.mWidth, videoMode.mHeight,
                & loupeCenter.data[0], & loupeCenter.data[1], & loupeSize.data[0], & loupeSize.data[1]);

    int loupeCenterX = loupeCenter.data[0];
    int loupeCenterY = loupeCenter.data[1];
    int loupeWidth = loupeSize.data[0];
    int loupeHeight = loupeSize.data[1];

    QCAR::RectangleInt detectionROI(loupeCenterX - (loupeWidth / 2), loupeCenterY - (loupeHeight /2),
            loupeCenterX + (loupeWidth / 2), loupeCenterY + (loupeHeight /2) );

    QCAR::Tracker* textTracker = QCAR::TrackerManager::getInstance().getTracker(QCAR::Tracker::TEXT_TRACKER);
    if(textTracker != 0)
        ((QCAR::TextTracker*)textTracker)->setRegionOfInterest(detectionROI, detectionROI, QCAR::TextTracker::REGIONOFINTEREST_UP_IS_9_HRS);


    viewportPosition_x =  (((int)(screenWidth  - config.mSize.data[0])) / (int) 2) + config.mPosition.data[0];
    viewportPosition_y =  (((int)(screenHeight - config.mSize.data[1])) / (int) 2) + config.mPosition.data[1];
    viewportSize_x = config.mSize.data[0];
    viewportSize_y = config.mSize.data[1];


    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_initApplicationNative(
                            JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_codered_ared_TextReco_initApplicationNative");
    
    // Store screen dimensions
    screenWidth = width;
    screenHeight = height;
        
    LOG("Java_com_codered_ared_TextReco_initApplicationNative finished");
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_deinitApplicationNative(
                                                        JNIEnv* env, jobject obj)
{
    LOG("Java_com_codered_ared_TextReco_deinitApplicationNative");
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_startCamera(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_startCamera");
    
    // Select the camera to open, set this to QCAR::CameraDevice::CAMERA_FRONT 
    // to activate the front camera instead.
    QCAR::CameraDevice::CAMERA camera = QCAR::CameraDevice::CAMERA_DEFAULT;

    // Initialize the camera:
    if (!QCAR::CameraDevice::getInstance().init(camera))
        return;

    // Configure the video background
    configureVideoBackground();

    // Select the default mode:
    if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                                QCAR::CameraDevice::MODE_DEFAULT))
        return;

    // Start the camera:
    if (!QCAR::CameraDevice::getInstance().start())
        return;

    // Start the tracker:
    LOG("Starting TextTracker...");

    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    // Start the text tracker
    QCAR::Tracker* textTracker = trackerManager.getTracker(QCAR::Tracker::TEXT_TRACKER);
    if(textTracker != 0)
        textTracker->start();

}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_stopCamera(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_stopCamera");

    // Stop the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    // Stop the text tracker
    QCAR::Tracker* textTracker = trackerManager.getTracker(QCAR::Tracker::TEXT_TRACKER);
    if(textTracker != 0)
        textTracker->stop();
    
    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextReco_setProjectionMatrix(JNIEnv *, jobject)
{
    LOG("Java_com_codered_ared_TextReco_setProjectionMatrix");

    // Cache the projection matrix:
    const QCAR::CameraCalibration& cameraCalibration =
                                QCAR::CameraDevice::getInstance().getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f, 2500.0f);
}

JNIEXPORT jboolean JNICALL
Java_com_codered_ared_TextReco_autofocus(JNIEnv*, jobject)
{
    return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_codered_ared_TextReco_setFocusMode(JNIEnv*, jobject, jint mode)
{
    int qcarFocusMode;

    switch ((int)mode)
    {
        case 0:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
            break;
        
        case 1:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
            break;
            
        case 2:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
            break;
            
        case 3:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
            break;
    
        default:
            return JNI_FALSE;
    }
    
    return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextRecoRenderer_initRendering(
                                                    JNIEnv* env, jobject obj)
{
    LOG("Java_com_codered_ared_TextRecoRenderer_initRendering");

    // Define clear color
    glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);
    
    lineShaderProgramID     = SampleUtils::createProgramFromBuffer(lineVertexShader,
                                                            lineFragmentShader);

    vertexHandle        = glGetAttribLocation(lineShaderProgramID,
                                                "vertexPosition");

    mvpMatrixHandle     = glGetUniformLocation(lineShaderProgramID,
                                                "modelViewProjectionMatrix");

    lineOpacityHandle   = glGetUniformLocation(lineShaderProgramID, "opacity");
    lineColorHandle   = glGetUniformLocation(lineShaderProgramID, "color");

    setOrthoMatrix(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, ROIOrthoProjMatrix);

}


JNIEXPORT void JNICALL
Java_com_codered_ared_TextRecoRenderer_updateRendering(
                        JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_codered_ared_TextRecoRenderer_updateRendering");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
}

// Support functionality for converting a Unicode string to an ASCII 7bit string
bool unicodeToAscii(const QCAR::Word& word, char*& asciiString)
{
    // Quick check if this string can be represented as ASCII. If the number
    // of code units in the string is not equal the number of characters, then
    // there are characters in this string that need two units to be represented
    // and thus cannot be represented in ASCII
    if (word.getLength() != word.getNumCodeUnits())
        return false;

    // Check if any individual character is outside the ASCII range
    const QCAR::UInt16* stringU = word.getStringU();
    for (int c = 0; c < word.getLength(); ++c)
        if (stringU[c] > 127)
            return false;

    // Create new string and convert
    char* stringA = new char[word.getLength()+1];
    for (int c = 0; c < word.getLength(); ++c)
        stringA[c] = (char) stringU[c];
    stringA[word.getLength()] = '\0';

    // Done
    asciiString = stringA;
    return true;
}



#ifdef __cplusplus
}
#endif
