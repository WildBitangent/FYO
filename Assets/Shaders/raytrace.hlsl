#define threadCountX 32
#define threadCountY 8
#define threadCount threadCountX * threadCountY
#define FLT_MAX 3.402823466e+38
#define EPSILON 1e-5
#define STACKSIZE 16

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
	uint triangleID;
	uint glassHits;
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

struct BVHNode
{
	float3 min;
	float pad0;

	float3 max;
	float pad1;
	
	int leftIndex;
	int rightIndex;
	bool isLeaf;
	float pad2;
};

//////////////////////////////////

RWTexture2D<float4> output : register(u0);
StructuredBuffer<BVHNode> tree : register(t0);
StructuredBuffer<float4> planeVertices : register(t1);
StructuredBuffer<float2> planeTexcoord : register(t2);
Buffer<uint4> planeIndices : register(t3);
Buffer<float4> lensVertices : register(t4);
Buffer<float4> lensNormals : register(t5);
Buffer<uint4> lensIndices : register(t6);
Texture2D planeTexture : register(t7);
SamplerState samplerState : register(s0);

//////////////////////////////////

inline Ray getPrimaryRay(float2 coord)
{
	float2 uv = coord * cam.pixelSize;
	return Ray::create(cam.pos, normalize(cam.ulc + uv.x * cam.horizontal - uv.y * cam.vertical));
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

bool rayAABBIntersection(Ray r, float3 minbox, float3 maxbox, out float t1, out float t2)
{
	float3 invdir = 1.0 / r.direction;

	float3 f = (maxbox - r.origin) * invdir;
	float3 n = (minbox - r.origin) * invdir;

	float3 tmax = max(f, n);
	float3 tmin = min(f, n);

	t2 = min(tmax.x, min(tmax.y, tmax.z));
	t1 = max(tmin.x, max(tmin.y, tmin.z));

	return t2 > t1;
	
	//return t0 <= t1 && t1 >= 0.0;
	//return (t1 >= t0) ? (t0 > 0.f ? t0 : t1) : -1.0;
}

bool asdasd(inout State state, float3 center, float radius, out float t1, out float t2)
{
	float3 sphereDir = center - state.ray.origin;
	float tca = dot(sphereDir, state.ray.direction);
	float d2 = dot(sphereDir, sphereDir) - tca * tca;
	float radius2 = radius * radius;
	
	if (d2 > radius2)
		return false;
	
	float thc = sqrt(radius2 - d2);
	t1 = tca - thc;
	t2 = tca + thc;
	
	return true;
}

void sphereBiconvex(inout State state, in float3 center1, in float3 center2, float radius, float3 minbox, float3 maxbox)
{
	float t1, t2, t3, t4, t5, t6;
	if (!asdasd(state, center1, radius, t1, t2) || !asdasd(state, center2, radius, t3, t4) || !rayAABBIntersection(state.ray, minbox, maxbox, t5, t6))
		return;
	
	t1 = (t1 < t2) ? t1 : t2;
	t3 = (t3 < t4) ? t3 : t4;
	
	//t1 = (t1 > t3) ? t1 : t3;
	
	float3 center = center1;
	if (t1 < t3)
	{
		t1 = t3;
		center = center2;
	}
	
	if (t1 < 0.f)
		return;
 
	state.hitPoint = state.ray.origin + t1 * state.ray.direction;
	float3 newDirection = glassSample(state.ray.direction, normalize(state.hitPoint - center));
	state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
	state.glassHits++;
	
	// inside glass
	asdasd(state, center1, radius, t1, t2);
	asdasd(state, center2, radius, t3, t4);
	
	t1 = (t1 > t2) ? t1 : t2;
	t3 = (t3 > t4) ? t3 : t4;
	
	//t1 = (t1 > t3) ? t1 : t3;
	
	center = center1;
	if (t1 > t3)
	{
		t1 = t3;
		center = center2;
	}
	
	//if (t1 < 0.f)
	//	return;
 
	state.hitPoint = state.ray.origin + t1 * state.ray.direction;
	newDirection = glassSample(state.ray.direction, normalize(state.hitPoint - center));
	state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
}

void sphereBiconcave(inout State state, in float3 center1, in float3 center2, float radius, float3 minbox, float3 maxbox)
{
	float t1, t2, t3, t4, t5, t6;
	bool box = rayAABBIntersection(state.ray, minbox, maxbox, t5, t6);
	if (!asdasd(state, center1, radius, t1, t2) || !asdasd(state, center2, radius, t3, t4) || !box)
		return;
	
	t1 = (t1 > t2) ? t1 : t2;
	t3 = (t3 > t4) ? t3 : t4;
	
	//t1 = (t1 > t3) ? t1 : t3;
	
	float3 center = center1;
	if (t1 > t3)
	{
		t1 = t3;
		center = center2;
	}
	
	//if (t1 < 0.f && !box)
	//	return;
 
	state.hitPoint = state.ray.origin + t1 * state.ray.direction;
	float3 newDirection = glassSample(state.ray.direction, normalize(state.hitPoint - center));
	state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
	state.glassHits++;
	
	// inside glass
	asdasd(state, center1, radius, t1, t2);
	asdasd(state, center2, radius, t3, t4);
	
	t1 = (t1 < t2) ? t1 : t2;
	t3 = (t3 < t4) ? t3 : t4;
	
	//t1 = (t1 > t3) ? t1 : t3;
	
	center = center1;
	if (t1 < t3)
	{
		t1 = t3;
		center = center2;
	}
	
	//if (t1 < 0.f)
	//	return;
 
	state.hitPoint = state.ray.origin + t1 * state.ray.direction;
	newDirection = glassSample(state.ray.direction, normalize(state.hitPoint - center));
	state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
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

[numthreads(threadCountX, threadCountY, 1)]
void main(uint3 gid : SV_DispatchThreadID, uint tid : SV_GroupIndex)
{
	State state; // todo this is shit
	state.ray = getPrimaryRay(gid.xy);
	state.hitPoint = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	state.baryCoord = float3(0, 0, 0);
	state.triangleID = 0;
	state.glassHits = 0;
	
	float4 color = float4(state.ray.direction.x, 1.0, 0.0, 1.0);
	
	sphereBiconcave(state, float3(0, 4, -10.5), float3(0, 4, 10.5), 10, float3(-2.5, 0.5, -1.5), float3(2.5, 7.5, 1.5));
	//sphereBiconvex(state, float3(0, 4, -8.5), float3(0, 4, 8.5), 10, float3(-2.5, 0.5, -1.5), float3(2.5, 7.5, 1.5));
	
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
	
	color *= pow(0.8, state.glassHits);
		
	output[gid.xy] = color;
	//output[gid.xy] = float4(0, 0, 0, 0);
}

	//while (rayBVHIntersection(state) < FLT_MAX)
	//{
	//	glassHit = true;
	//	uint3 idx = lensIndices[state.triangleID + nTrianglesLens];
	//	float3 n0 = lensNormals[idx.x];
	//	float3 n1 = lensNormals[idx.y];
	//	float3 n2 = lensNormals[idx.z];

	//	float3 normal = n0 * state.baryCoord.x + n1 * state.baryCoord.y + n2 * state.baryCoord.z;
	//	float3 newDirection = glassSample(state.ray.direction, normal);
	//	state.ray = Ray::create(state.hitPoint + newDirection * 1e-8, newDirection);
	//}
	
	
	
	//for (uint x = 0; x < 1; x++)
	//{
	//	float lastHit = FLT_MAX;
	//	uint lastIdx = 0;
	//	for (uint i = 0; i < nTrianglesLens; i++)
	//	{
	//		uint3 idx = lensIndices[i];
	//		if (rayTriangleIntersection(state, lensVertices[idx.x].xyz, lensVertices[idx.y].xyz, lensVertices[idx.z].xyz, lastHit))
	//			lastIdx = i;
	//	}
		
	//	// advance ray
	//	if (lastHit < FLT_MAX)
	//	{
	//		glassHit = true;
	//		uint3 idx = lensIndices[lastIdx + nTrianglesLens];
	//		float3 n0 = lensNormals[idx.x];
	//		float3 n1 = lensNormals[idx.y];
	//		float3 n2 = lensNormals[idx.z];

	//		float3 normal = n0 * state.baryCoord.x + n1 * state.baryCoord.y + n2 * state.baryCoord.z;
	//		float3 newDirection = glassSample(state.ray.direction, normal);
			
	//		state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
	//	}
	//}
	
	//float3 normal;
	//float lastHit = FLT_MAX;
	//if (raySphereIntersection(state, normal, float3(0, 0, 0), 2, lastHit))
	//{
	//	float3 newDirection = glassSample(state.ray.direction, normal);
	//	state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
		
	//	if (raySphereIntersection(state, normal, float3(0, 0, 0), 5, lastHit))
	//	{
	//		float3 newDirection = glassSample(state.ray.direction, normal);
	//		state.ray = Ray::create(state.hitPoint + newDirection * 1e-5, newDirection);
	//	}
	//}