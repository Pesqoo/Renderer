struct PSIn
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

float4 PSMain(PSIn i) : SV_TARGET
{
    return i.color;
}
