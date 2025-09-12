#vertex
#version 330 core

layout(location = 0) in vec3 a_vertexPos;
layout(location = 1) in vec2 a_texCoord;

uniform mat4 u_ProjectionViewMatrix;
uniform mat4 u_ModelMatrix;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_texCoord;
	gl_Position = u_ProjectionViewMatrix * u_ModelMatrix * vec4(a_vertexPos, 1.0);
}

#fragment
#version 330 core

layout(location = 0) out vec4 v_fragColor;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	v_fragColor = texture(u_Texture, v_TexCoord);
}