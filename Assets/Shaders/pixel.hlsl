struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D Texture;
SamplerState Sampler;

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = Texture.Sample(Sampler, input.texCoord);
	if ((input.texCoord.x >= 0.49 && input.texCoord.x <= 0.51 && input.texCoord.y >= 0.499 && input.texCoord.y <= 0.501) ||
		(input.texCoord.y >= 0.49 && input.texCoord.y <= 0.51 && input.texCoord.x >= 0.499 && input.texCoord.x <= 0.501))
		color = (color + float4(0, 1, 0, 0)) / 2;
	
	return color;
}