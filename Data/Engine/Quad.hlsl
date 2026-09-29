cbuffer QuadConstants : register(b0)
{
    row_major float4x4 ViewProjection;
};

Texture2D QuadTexture : register(t0);
SamplerState QuadSampler : register(s0);

struct VSInput
{
    float2 Position : POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // NOTE(saeb): z = 0 and w = 1: a flat 2D point that the matrix fully places.
    output.Position = mul(float4(input.Position, 0.0, 1.0), ViewProjection);
    output.UV = input.UV;
    output.Color = input.Color;

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return QuadTexture.Sample(QuadSampler, input.UV) * input.Color;
}