#version 330 core

in vec3 a_FColor;
in vec2 a_TexCoord;
out vec4 a_FragmentColor;

uniform sampler2D u_Texture;

void main()
{
	vec4 texColor = texture(u_Texture, a_TexCoord);
	a_FragmentColor = texColor;
}