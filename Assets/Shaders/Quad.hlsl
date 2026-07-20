cbuffer Constants : register(b0)
{
    row_major float4x4 Projection;
};

struct VSInput
{
    float2 Corner : POSITION;   // Per-vertex: unit quad corner
    float4 Rect   : RECT;       // Per-instance: x, y, width, height
    float4 UVRect : UVRECT;     // Per-instance: u0, v0, u1, v1
    float4 Color  : COLOR;      // Per-instance
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    float2 position = input.Rect.xy + input.Corner * input.Rect.zw;
    float2 uv = lerp(input.UVRect.xy, input.UVRect.zw, input.Corner);

    PSInput output;
    output.Position = mul(float4(position, 0.0f, 1.0f), Projection);
    output.UV = uv;
    output.Color = input.Color;

    return output;
}

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    return Texture.Sample(Sampler, input.UV) * input.Color;
}

float4 PSMainAlpha(PSInput input) : SV_TARGET
{
    float coverage = Texture.Sample(Sampler, input.UV).r;
    return float4(1.0f, 1.0f, 1.0f, coverage) * input.Color;
}