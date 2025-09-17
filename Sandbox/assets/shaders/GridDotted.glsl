#vertex
#version 330 core

layout(location = 0) in vec3 a_vertexPos;

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

uniform vec3 u_GridColor   = vec3(0.8, 0.8, 0.8);
uniform vec3 u_BGColor     = vec3(0.95, 0.95, 0.95);
uniform float u_Spacing    = 1.0;   // Distance between dots.
uniform float u_DotRadius  = 0.05;  // Dot size (in world units).

void main()
{
    // Get position within one grid cell.
    vec2 cell = mod(v_WorldPos, u_Spacing);

    // Center of the grid cell (dot position).
    vec2 dotCenter = vec2(u_Spacing * 0.5);

    // Distance from this fragment to the dot center.
    float dist = length(cell - dotCenter);

    // Smooth circle mask.
    float dotMask = smoothstep(u_DotRadius, u_DotRadius * 0.8, dist);

    vec3 color = mix(u_GridColor, u_BGColor, dotMask);
    FragColor = vec4(color, 1.0);
}