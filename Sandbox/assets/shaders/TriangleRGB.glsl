#vertex
#version 330 core
			
layout(location = 0) in vec3 a_vertexPos;
layout(location = 1) in vec3 a_vertexColor;

uniform mat4 u_ProjectionViewMatrix;

out vec3 v_vertexColor;
			
void main()
{
	gl_Position = u_ProjectionViewMatrix * vec4(a_vertexPos, 1.0);
	v_vertexColor = a_vertexColor;
}

#fragment
#version 330 core
			
in vec3 v_vertexColor;
out vec4 v_fragColor; 			

void main()
{
	v_fragColor = vec4(v_vertexColor, 1.0);
}