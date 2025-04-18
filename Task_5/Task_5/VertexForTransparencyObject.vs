cbuffer MatrixBuffer : register(b0)
{
    matrix m;
};

cbuffer CameraBuffer : register(b1)
{
    matrix vp;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_Position;
};

VSOutput main(VSInput vertex)
{
    VSOutput output;
    float4 pos = float4(vertex.pos, 1.0);
    output.pos = mul(pos, m);
    output.pos = mul(output.pos, vp);
    return output;
}