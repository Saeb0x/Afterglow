// NOTE(saeb): Signed distance field text. The atlas stores, for each pixel, the distance to the glyph's edge: 0.5 on the edge, higher inside, lower outside. The GPU blends those distances when the glyph is scaled, and this shader only decides which side of the edge each pixel is on, so edges stay crisp at any size.

Texture2D Atlas : register(t0);
SamplerState AtlasSampler : register(s0);

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

float4 PSMain(PSInput input) : SV_Target
{
    float distance = Atlas.Sample(AtlasSampler, input.UV).r;

    // Half a screen pixel of distance on each side of the edge, so the edge is a one-pixel smooth rim at every size. The floor avoids a zero width.
    float width = max(fwidth(distance) * 0.5, 0.0001);
    float coverage = smoothstep(0.5 - width, 0.5 + width, distance);

    // The renderer already premultiplied the colour; scaling by coverage keeps it premultiplied.
    return input.Color * coverage;
}