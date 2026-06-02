#version 120

uniform mat4 projectorMVP;

varying vec4 vColor;
varying vec3 vNormal;
varying vec3 vWorldPos;
varying vec4 vProjTexCoord;

void main() {
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
    vColor = gl_Color;
    vNormal = gl_Normal;
    vWorldPos = gl_Vertex.xyz;
    vProjTexCoord = projectorMVP * gl_Vertex;
}
