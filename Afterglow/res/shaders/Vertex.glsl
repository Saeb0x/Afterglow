#version 330 core

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_UVs;

out vec3 a_FColor;
out vec2 a_TexCoord;

uniform mat4 u_ProjectionViewMatrix;

void main()
{
	gl_Position = u_ProjectionViewMatrix * a_Pos;
	a_FColor = a_Color;
	a_TexCoord = a_UVs;
}