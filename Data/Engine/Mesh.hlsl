cbuffer MeshConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    row_major float4x4 World; // The mesh's metres to world metres
    float4 Color;
    float4 ToSun; // xyz: the unit direction from any surface toward the sun
    float4 SunColor;
    float4 AmbientColor; // Light that reaches every surface, even the ones facing away from the sun
};

struct VSInput
{
    float3 Position : ATTRIB0;
    float3 Normal : ATTRIB1;
};

struct PSInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // NOTE(saeb): Into the world, then through the camera. The normal turns with the mesh too; w = 0 would ignore the move, and the 3x3 part does exactly that.
    float4 worldPosition = mul(float4(input.Position, 1.0), World);
    output.Position = mul(worldPosition, ViewProjection);
    output.Normal = mul(input.Normal, (float3x3)World);

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    // NOTE(saeb): Lambert lighting. A surface facing the sun head-on catches all of its light; tilted away, it catches less, by the cosine of the angle, which is the dot product of the two unit directions. Facing away gives a negative dot, clamped to no sunlight at all.
    float3 normal = normalize(input.Normal);
    float facing = saturate(dot(normal, ToSun.xyz));

    float3 light = AmbientColor.rgb + SunColor.rgb * facing;
    return float4(Color.rgb * light, 1.0);
}