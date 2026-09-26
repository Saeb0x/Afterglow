cbuffer QuadConstants : register(b0)
{
    float4 VisibleArea; // x, y, width, height: the part of design space the window shows
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

    // Design units (top-left origin, y down) -> clip space (center origin, y up): an orthographic projection of the visible area onto the whole window.
    float2 clip = ((input.Position - VisibleArea.xy) / VisibleArea.zw) * float2(2.0, -2.0) + float2(-1.0, 1.0);

    output.Position = float4(clip, 0.0, 1.0);
    output.UV = input.UV;
    output.Color = input.Color;

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return QuadTexture.Sample(QuadSampler, input.UV) * input.Color;
}