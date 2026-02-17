cbuffer VSConstants : register(b0)
{
    float4x4 gWorldViewProj;
};

struct VSIn
{
    float3 pos : POSITION;
    float4 color : COLOR0;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

VSOut VSMain(VSIn i)
{
    VSOut o;
    o.pos = mul(float4(i.pos, 1.0f), gWorldViewProj);
    o.color = i.color;
    return o;
}
