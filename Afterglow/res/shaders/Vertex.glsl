#version 330 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_UVs;
layout (location = 3) in float a_TextureID;

out vec4 a_FColor;
out vec2 a_FTextureCoords;
out float a_FTextureID;

uniform mat4 u_ProjectionViewMatrix;

void main()
{
	gl_Position = u_ProjectionViewMatrix * vec4(a_Pos, 1.0);
	a_FColor = a_Color;
	a_FTextureCoords = a_UVs;
	a_FTextureID = a_TextureID;
}