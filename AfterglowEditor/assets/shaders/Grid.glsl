#vertex
#version 330 core

layout(location = 0) in vec3 a_vertexPos;
layout(location = 1) in vec2 a_texCoord;

uniform mat4 u_ProjectionViewMatrix;
uniform mat4 u_ModelMatrix;

out vec2 v_WorldPos;

void main()
{
    vec4 worldPos = u_ModelMatrix * vec4(a_vertexPos, 1.0);
    v_WorldPos = worldPos.xy;

    gl_Position = u_ProjectionViewMatrix * worldPos;
}

#fragment
#version 330 core

in vec2 v_WorldPos;
out vec4 FragColor;

uniform vec3 u_GridColor = vec3(0.8, 0.8, 0.8);
uniform vec3 u_BGColor   = vec3(0.95, 0.95, 0.95);
uniform float u_Spacing  = 1.0;   // Grid spacing in world units.
uniform float u_Thickness = 0.02; // Line thickness in world units.

void main()
{
    // Distance from nearest vertical and horizontal grid line.
    float distX = abs(mod(v_WorldPos.x, u_Spacing) - u_Spacing * 0.5);
    float distY = abs(mod(v_WorldPos.y, u_Spacing) - u_Spacing * 0.5);

    // Pixel is part of a line if it's close enough to any axis-aligned grid line.
    float line = step(min(distX, distY), u_Thickness);

    vec3 color = mix(u_BGColor, u_GridColor, line);
    FragColor = vec4(color, 1.0);
}