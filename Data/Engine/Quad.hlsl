cbuffer QuadConstants : register(b0)
{
    row_major float4x4 ViewProjection;
};

Texture2D QuadTexture : register(t0);
SamplerState QuadSampler : register(s0);

// NOTE(saeb): What to do with a quad, per vertex; Renderer2D.cpp writes the same values.
#define QUAD_SOLID 0.0
#define QUAD_TEXTURED 1.0
#define QUAD_TEXT 2.0

struct VSInput
{
    float2 Position : ATTRIB0;
    float2 UV : ATTRIB1;
    float4 Color : ATTRIB2;
    float Mode : ATTRIB3;
};

struct PSInput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
    nointerpolation float Mode : MODE; // The same on all four corners; nointerpolation keeps it exact
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // NOTE(saeb): z = 0 and w = 1: a flat 2D point that the matrix fully places.
    output.Position = mul(float4(input.Position, 0.0, 1.0), ViewProjection);
    output.UV = input.UV;
    output.Color = input.Color;
    output.Mode = input.Mode;

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    // NOTE(saeb): Sample and take derivatives before choosing a mode: fwidth needs every pixel in the 2x2 block to run it, which branching first wouldn't guarantee.
    float4 texel = QuadTexture.Sample(QuadSampler, input.UV);

    // NOTE(saeb): Signed distance field text. The atlas stores, for each pixel, the distance to the glyph's edge: 0.5 on the edge, higher inside, lower outside. The GPU blends those distances when the glyph is scaled, and this only decides which side of the edge each pixel is on, so edges stay crisp at any size. Half a screen pixel of distance on each side of the edge makes a one-pixel smooth rim at every size; the floor avoids a zero width.
    float distance = texel.r;
    float width = max(fwidth(distance) * 0.5, 0.0001);
    float coverage = smoothstep(0.5 - width, 0.5 + width, distance);

    // NOTE(saeb): The renderer premultiplies the colour and the cooker premultiplies textures, so every result stays premultiplied.
    if(input.Mode > (QUAD_TEXTURED + QUAD_TEXT) * 0.5)
    {
        return input.Color * coverage;
    }

    if(input.Mode > (QUAD_SOLID + QUAD_TEXTURED) * 0.5)
    {
        return texel * input.Color;
    }

    return input.Color;
}