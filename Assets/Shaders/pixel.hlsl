struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D Texture;
SamplerState Sampler;

float4 main(PS_INPUT input) : SV_TARGET
{
	return Texture.Sample(Sampler, input.texCoord);
}