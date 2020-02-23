#define threadCountX 32
#define threadCountY 8
#define threadCount threadCountX * threadCountY
#define FLT_MAX 3.402823466e+38
#define EPSILON 1e-8

struct Ray
{
	float3 origin;
	float3 direction;

	static Ray create(float3 o, float3 d)
	{
		Ray r;
		r.origin = o;
		r.direction = d;
		return r;
	}
};

struct Camera
{
	float3 pos;
	float3 ulc;
	float3 horizontal;
	float3 vertical;
	float2 pixelSize;
};

struct State
{
	Ray ray;
	float3 hitPoint;
	float3 baryCoord;
};

cbuffer Cam : register(b0)
{
	Camera cam;
};

cbuffer Model : register(b1)
{
	uint nTrianglesPlane;
	uint nTrianglesLens;
	float lensIor;
};

//////////////////////////////////

RWTexture2D<float4> output : register(u0);
StructuredBuffer<float4> planeVertices : register(t0);
StructuredBuffer<float2> planeTexcoord : register(t1);
Buffer<uint4> planeIndices : register(t2);
StructuredBuffer<float4> lensVertices : register(t3);
StructuredBuffer<float4> lensNormals : register(t4);
Buffer<uint4> lensIndices : register(t5);
Texture2D planeTexture : register(t6);
SamplerState samplerState : register(s0);

//////////////////////////////////

inline Ray getPrimaryRay(float2 coord)
{
	float2 uv = coord * cam.pixelSize;
	return Ray::create(cam.pos, normalize(cam.ulc + uv.x * cam.horizontal - uv.y * cam.vertical));
}

bool rayTriangleIntersection(inout State state, float3 v0, float3 v1, float3 v2, inout float lastT)
{
	float3 e1 = v1 - v0;
	float3 e2 = v2 - v0;
	
	float3 pvec = cross(state.ray.direction, e2);
	float det = dot(e1, pvec);

	if (det > -EPSILON && det < EPSILON)
		return false;
		
	float invDet = 1.0 / det;
	float3 tvec = state.ray.origin - v0;
	float u = dot(tvec, pvec) * invDet;
	
	if (u < 0.0f || u > 1.0f)
		return false;
		
	float3 qvec = cross(tvec, e1);
	float v = dot(state.ray.direction, qvec) * invDet;
	
	if (v < 0.0 || u + v > 1.0)
		return false;
	
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = dot(e2, qvec) * invDet;
	
	if (t > EPSILON && t < 1 / EPSILON) // ray intersection
	{
		float3 pp = state.ray.origin + state.ray.direction * t;

		if (t < lastT)
		{
			lastT = t;
			state.hitPoint = pp;
			state.baryCoord = float3(1 - u - v, u, v);
			return true;
		}
	}
	return false;
}

float4 textureTriangle(float3 baryCoord, in uint index)
{
	uint3 idx = planeIndices[index + (nTrianglesPlane << 1)];
	
	float2 t0 = planeTexcoord[idx.x];
	float2 t1 = planeTexcoord[idx.y];
	float2 t2 = planeTexcoord[idx.z];

	float2 texCoord = t0 * baryCoord.x + t1 * baryCoord.y + t2 * baryCoord.z;

	return planeTexture.SampleLevel(samplerState, texCoord, 0);
}

float3 schlickFresnel(float3 r0, float theta)
{
	float m = saturate(1.0 - theta);
	float m2 = m * m;
	return r0 - (1 - r0) * m2 * m2 * m;
}

float3 glassSample(in float3 rayDirection, in float3 normal)
{
	float3 normalF = dot(normal, rayDirection) <= 0.0 ? normal : normal * -1.0; // todo wtf is this

	// refraction
	float n1 = 1.0;
	float n2 = lensIor;

	//float r0 = (n1 - n2) / (n1 + n2);
	//r0 *= r0;
	
	//float theta = dot(-state.ray.direction, normal);
	//float probability = schlickFresnel(r0, theta);

	// decide where do we go, inside or outside
	float refractFactor = dot(normal, normalF) > 0.0 ? (n1 / n2) : (n2 / n1); 
	float3 transDirection = normalize(refract(rayDirection, normalF, refractFactor));
	
	//float cos2t = 1.0 - refractFactor * refractFactor * (1.0 - theta * theta);
	//if (cos2t < 0.0 || rand() < probability)
	//	return normalize(reflect(state.ray.direction, normal));
        
	return transDirection;
}

[numthreads(threadCountX, threadCountY, 1)]
void main(uint3 gid : SV_DispatchThreadID, uint tid : SV_GroupIndex)
{
	State state; // todo this is shit
	state.ray = getPrimaryRay(gid.xy);
	state.hitPoint = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	state.baryCoord = float3(0, 0, 0);
	
	bool glassHit = false;
	float4 color = float4(state.ray.direction.x, 1.0, 0.0, 1.0);
	
	for (uint x = 0; x < 2; x++)
	{
		float lastHit = FLT_MAX;
		uint lastIdx = 0;
		for (uint i = 0; i < nTrianglesLens; i++)
		{
			uint3 idx = lensIndices[i];
			if (rayTriangleIntersection(state, lensVertices[idx.x].xyz, lensVertices[idx.y].xyz, lensVertices[idx.z].xyz, lastHit))
				lastIdx = i;
		}
		
		// advance ray
		if (lastHit < FLT_MAX)
		{
			glassHit = true;
			uint3 idx = lensIndices[lastIdx + nTrianglesLens];
			float3 n0 = lensNormals[idx.x];
			float3 n1 = lensNormals[idx.y];
			float3 n2 = lensNormals[idx.z];

			float3 normal = n0 * state.baryCoord.x + n1 * state.baryCoord.y + n2 * state.baryCoord.z;
			float3 newDirection = glassSample(state.ray.direction, normal);
			
			state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
		}
	}
	
	for (uint i = 0; i < nTrianglesPlane; i++)
	{
		float lastHit = FLT_MAX;
		uint3 idx = planeIndices[i];
		
		rayTriangleIntersection(state, planeVertices[idx.x].xyz, planeVertices[idx.y].xyz, planeVertices[idx.z].xyz, lastHit);
			
		if (lastHit < FLT_MAX)
		{
			color = textureTriangle(state.baryCoord, i);
			break;
		}
	}
	
	if (glassHit)
		color *= 0.8;
		
	output[gid.xy] = color;
	//output[gid.xy] = float4(0, 0, 0, 0);
}