#version 330 core

out vec4 a_FragmentColor;

uniform vec4 u_FColor;

void main()
{
	a_FragmentColor = u_FColor;
}