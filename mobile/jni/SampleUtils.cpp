#include "SampleUtils.h"

#include <math.h>
#include <stdlib.h>

#ifdef USE_OPENGL_ES_1_1
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif


void
SampleUtils::printMatrix(const float* mat)
{
    for(int r=0; r<4; r++,mat+=4)
        LOG("%7.3f %7.3f %7.3f %7.3f", mat[0], mat[1], mat[2], mat[3]);
}


void
SampleUtils::checkGlError(const char* operation)
{ 
    for (GLint error = glGetError(); error; error = glGetError())
        LOG("after %s() glError (0x%x)", operation, error);
}


void
SampleUtils::translatePoseMatrix(float x, float y, float z, float* matrix)
{
    // Sanity check
    if (!matrix)
        return;

    // matrix * translate_matrix
    matrix[12] += 
        (matrix[0] * x + matrix[4] * y + matrix[8]  * z);
        
    matrix[13] += 
        (matrix[1] * x + matrix[5] * y + matrix[9]  * z);
        
    matrix[14] += 
        (matrix[2] * x + matrix[6] * y + matrix[10] * z);
        
    matrix[15] += 
        (matrix[3] * x + matrix[7] * y + matrix[11] * z);
}


void
SampleUtils::rotatePoseMatrix(float angle, float x, float y, float z,
                              float* matrix)
{
    // Sanity check
    if (!matrix)
        return;

    float rotate_matrix[16];
    SampleUtils::setRotationMatrix(angle, x, y, z, rotate_matrix);
        
    // matrix * scale_matrix
    SampleUtils::multiplyMatrix(matrix, rotate_matrix, matrix);
}


void
SampleUtils::scalePoseMatrix(float x, float y, float z, float* matrix)
{
    // Sanity check
    if (!matrix)
        return;

    // matrix * scale_matrix
    matrix[0]  *= x;
    matrix[1]  *= x;
    matrix[2]  *= x;
    matrix[3]  *= x;
                     
    matrix[4]  *= y;
    matrix[5]  *= y;
    matrix[6]  *= y;
    matrix[7]  *= y;
                     
    matrix[8]  *= z;
    matrix[9]  *= z;
    matrix[10] *= z;
    matrix[11] *= z;
}


void
SampleUtils::multiplyMatrix(float *matrixA, float *matrixB, float *matrixC)
{
    int i, j, k;
    float aTmp[16];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            aTmp[j * 4 + i] = 0.0;

            for (k = 0; k < 4; k++)
                aTmp[j * 4 + i] += matrixA[k * 4 + i] * matrixB[j * 4 + k];
        }
    }

    for (i = 0; i < 16; i++)
        matrixC[i] = aTmp[i];
}


void
SampleUtils::setRotationMatrix(float angle, float x, float y, float z, 
    float *matrix)
{
    double radians, c, s, c1, u[3], length;
    int i, j;

    radians = (angle * M_PI) / 180.0;

    c = cos(radians);
    s = sin(radians);

    c1 = 1.0 - cos(radians);

    length = sqrt(x * x + y * y + z * z);

    u[0] = x / length;
    u[1] = y / length;
    u[2] = z / length;

    for (i = 0; i < 16; i++)
        matrix[i] = 0.0;

    matrix[15] = 1.0;

    for (i = 0; i < 3; i++)
    {
        matrix[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
        matrix[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
    }

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
            matrix[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0);
    }
}


unsigned int
SampleUtils::initShader(unsigned int shaderType, const char* source)
{
#ifdef USE_OPENGL_ES_2_0    
    GLuint shader = glCreateShader((GLenum)shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOG("Could not compile shader %d: %s", 
                        shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
#else
    return 0;
#endif
}


unsigned int
SampleUtils::createProgramFromBuffer(const char* vertexShaderBuffer,
                                     const char* fragmentShaderBuffer)
{
#ifdef USE_OPENGL_ES_2_0    

    GLuint vertexShader = initShader(GL_VERTEX_SHADER, vertexShaderBuffer);
    if (!vertexShader)
        return 0;    

    GLuint fragmentShader = initShader(GL_FRAGMENT_SHADER,
                                        fragmentShaderBuffer);
    if (!fragmentShader)
        return 0;

    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        
        glAttachShader(program, fragmentShader);
        checkGlError("glAttachShader");
        
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char* buf = (char*) malloc(bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOG("Could not link program: %s", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
#else
    return 0;
#endif
}


// Transforms a screen pixel to a pixel onto the camera image,
// taking into account e.g. cropping of camera image to fit different aspect ratio screen.
// for the camera dimensions, the width is always bigger than the height (always landscape orientation)
// Top left of screen/camera is origin
void SampleUtils::screenCoordToCameraCoord(int screenX, int screenY, int screenDX, int screenDY,
        int screenWidth, int screenHeight, int cameraWidth, int cameraHeight,
        int * cameraX, int* cameraY, int * cameraDX, int * cameraDY)
{
    bool isPortraitMode = (screenWidth < screenHeight);
    float videoWidth, videoHeight;
    videoWidth = (float)cameraWidth;
    videoHeight = (float)cameraHeight;

//  LOG("@@ screenCoordToCameraCoord X=%d, Y=%d DX=%d, DY=%d screen: w=%d h=%d camera w=%d, h=%d", screenX, screenY, screenDX, screenDY,
//       screenWidth, screenHeight, cameraWidth, cameraHeight);

    if (isPortraitMode)
    {
        // the width and height of the camera are always
        // based on a landscape orientation

        // as the camera coordinates are always in landscape
        // we convert the inputs into a landscape based coordinate system
        int tmp = screenX;
        screenX = screenY;
        screenY = screenWidth - tmp;

        tmp = screenDX;
        screenDX = screenDY;
        screenDY = tmp;

        tmp = screenWidth;
        screenWidth = screenHeight;
        screenHeight = tmp;
    }
    else
    {
        videoWidth = (float)cameraWidth;
        videoHeight = (float)cameraHeight;
    }

    float videoAspectRatio = videoHeight / videoWidth;
    float screenAspectRatio = (float) screenHeight / (float) screenWidth;

    float scaledUpX;
    float scaledUpY;
    float scaledUpVideoWidth;
    float scaledUpVideoHeight;

    if (videoAspectRatio < screenAspectRatio)
    {
        // the video height will fit in the screen height
        scaledUpVideoWidth = (float)screenHeight / videoAspectRatio;
        scaledUpVideoHeight = screenHeight;
        scaledUpX = (float)screenX + ((scaledUpVideoWidth - (float)screenWidth) / 2.0f);
        scaledUpY = (float)screenY;
    }
    else
    {
        // the video width will fit in the screen width
        scaledUpVideoHeight = (float)screenWidth * videoAspectRatio;
        scaledUpVideoWidth = screenWidth;
        scaledUpY = (float)screenY + ((scaledUpVideoHeight - (float)screenHeight)/2.0f);
        scaledUpX = (float)screenX;
    }

    if (cameraX)
    {
        *cameraX = (int)((scaledUpX / (float)scaledUpVideoWidth) * videoWidth);
    }

    if (cameraY)
    {
        *cameraY = (int)((scaledUpY / (float)scaledUpVideoHeight) * videoHeight);
    }

    if (cameraDX)
    {
        *cameraDX = (int)(((float)screenDX / (float)scaledUpVideoWidth) * videoWidth);
    }

    if (cameraDY)
    {
        *cameraDY = (int)(((float)screenDY / (float)scaledUpVideoHeight) * videoHeight);
    }
}

