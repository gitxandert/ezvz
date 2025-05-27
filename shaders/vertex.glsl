// vertex.glsl
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;

uniform mat4 uProjection;
uniform mat4 uModel;

out vec2 vUV;
void main(){
    vUV = aUV;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}