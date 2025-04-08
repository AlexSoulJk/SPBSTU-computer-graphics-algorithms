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
    float2 tex : TEXCOORD;
};

struct VSOutput {
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
};

VSOutput main(VSInput vertex) {
    VSOutput output;
    float4 pos = float4(vertex.pos, 1.0);
    output.pos = mul(mul(pos, m), vp);
    output.tex = vertex.tex;
    return output;
}




