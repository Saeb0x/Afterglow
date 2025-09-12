#vertex
#version 330 core

layout(location = 0) in vec3 a_vertexPos;

uniform mat4 u_ProjectionViewMatrix;
uniform mat4 u_ModelMatrix;

void main()
{
	gl_Position = u_ProjectionViewMatrix * u_ModelMatrix * vec4(a_vertexPos, 1.0);
}

#fragment
#version 330 core

layout(location = 0) out vec4 v_fragColor;

uniform vec4 u_Color;

void main()
{
	v_fragColor = u_Color;
}