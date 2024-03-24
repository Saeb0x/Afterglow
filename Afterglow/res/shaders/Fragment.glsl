#version 330 core

in vec3 a_FColor;
out vec4 a_FragmentColor;

void main()
{
	a_FragmentColor = vec4(a_FColor, 1.0f);
}