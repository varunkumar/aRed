#ifndef _QCAR_LINE_SHADERS_H_
#define _QCAR_LINE_SHADERS_H_

static const char* lineVertexShader = " \
  \
attribute vec4 vertexPosition; \
 \
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
} \
";


static const char* lineFragmentShader = " \
 \
precision mediump float; \
 \
uniform float opacity; \
 \
uniform vec3 color; \
 \
void main() \
{ \
   gl_FragColor = vec4(color.r, color.g, color.b, opacity); \
} \
";

#endif // _QCAR_LINE_SHADERS_H_
