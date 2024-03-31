#version 330 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec4 a_Color;

out vec4 a_FColor;

uniform mat4 u_ProjectionViewMatrix;

void main()
{
	gl_Position = u_ProjectionViewMatrix * vec4(a_Pos, 1.0);
	a_FColor = a_Color;
}