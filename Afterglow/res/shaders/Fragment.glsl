#version 330 core

in vec4 a_FColor;
in vec2 a_FTextureCoords;
in float a_FTextureID;

out vec4 a_FragmentColor;

uniform sampler2D u_Textures[8];

void main()
{
	if(a_FTextureID > 0)
	{
		int id = int(a_FTextureID);
		a_FragmentColor = a_FColor * texture(u_Textures[id], a_FTextureCoords);
	} else {
		a_FragmentColor = a_FColor;
	}
}