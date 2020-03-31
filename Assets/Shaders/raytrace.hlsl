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
	uint glassHits;
};

cbuffer Cam : register(b0)
{
	Camera cam;
};

cbuffer Model : register(b1)
{
	uint nTrianglesPlane;
	uint lensCount;
	float lensIor;
	uint raysCount;

	float3 lensMinBox;
	float lineWidth;
	
	float3 lensMaxBox;
	uint chromaticAberration;
};

struct LensStruct
{
	float3 center1;
	float radius1;

	float3 center2;
	float radius2;
	
	float3 minbox;
	float pad0;
	
	float3 maxbox;
	uint type;

	float4 pad1;
	float4 pad2;
};

//////////////////////////////////

RWTexture2D<float4> output : register(u0);
StructuredBuffer<LensStruct> lens : register(t0);
Buffer<float4> planeVertices : register(t1);
Buffer<float2> planeTexcoord : register(t2);
Buffer<uint4> planeIndices : register(t3);
Texture2D planeTexture : register(t4);
StructuredBuffer<Ray> rays : register(t5);
SamplerState samplerState : register(s0);

//////////////////////////////////

groupshared uint offsets[33]; // offsets to points, max 32 (first one is dummy) (based on app)
groupshared float3 points[512];

groupshared float3 raysMin;
groupshared float3 raysMax;

static uint index;

//////////////////////////////////


inline Ray getPrimaryRay(float2 coord)
{
	float2 uv = coord * cam.pixelSize;
	return Ray::create(cam.pos, normalize(cam.ulc + uv.x * cam.horizontal - uv.y * cam.vertical));
}

inline void addPoint(inout uint counter, in float3 pp, in float radius)
{
	points[counter++] = pp;
	raysMin = min(raysMin, pp - radius);
	raysMax = max(raysMax, pp + radius);
}

float3 glassSample(in float3 rayDirection, in float3 normal, in bool rayToMat)
{
	float n1 = 1.0;
	float n2 = lensIor;
	if (chromaticAberration)
	{
		// red 1
		// green 1.004088339948448
		// blue 1.005122020713372
		if (index == 1) n2 *= 1.004088339948448;
		else if (index == 2) n2 *= 1.005122020713372;
	}

	// decide where do we go, inside or outside
	float refractFactor = (rayToMat ? n1 / n2 : n2 / n1);
	float3 transDirection = normalize(refract(rayDirection, normal, refractFactor));

	return transDirection;
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

//////////////////////////////////

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
			//state.hitPoint = float3(1 - u - v, u, v);
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

	return t2 > t1 && t2 >= 0.0;
}

bool raySphereIntersection(inout State state, float3 center, float radius, out float t1, out float t2)
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

bool rayCapsuleIntersection(in Ray r, float3 start, float3 end, float radius)
{
	float3 ba = end - start;
	float3 oa = r.origin - start;

	float baba = dot(ba, ba);
	float bard = dot(ba, r.direction);
	float baoa = dot(ba, oa);
	float rdoa = dot(r.direction, oa);
	float oaoa = dot(oa, oa);

	float a = baba - bard * bard;
	float b = baba * rdoa - baoa * bard;
	float c = baba * oaoa - baoa * baoa - radius * radius * baba;
	float h = b * b - a * c;
	
	if (h > 0.0)
	{
		float t = (-b - sqrt(h)) / a;
		float y = baoa + t * bard;
		
		// body
		if (y > 0.0 && y < baba && t > 0.0)
			return true;
			//return t;
		
		// caps
		float3 oc = (y <= 0.0) ? oa : r.origin - end;
		b = dot(r.direction, oc);
		c = dot(oc, oc) - radius * radius;
		h = b * b - c;
		if (h > 0.0 && -b - sqrt(h) > 0.0)
			return true;
			//return -b - sqrt(h);
	}

	return false;
}

//////////////////////////////////

void sphereBiconvex(inout State state, in float3 center1, in float3 center2, float radius1, float radius2, float3 minbox, float3 maxbox)
{
	float t1, t2, t3, t4, t5, t6;
	if (!raySphereIntersection(state, center1, radius1, t1, t2) || 
		!raySphereIntersection(state, center2, radius2, t3, t4) || 
		!rayAABBIntersection(state.ray, minbox, maxbox, t5, t6))
		return;

	state.glassHits++;
	if (state.ray.direction.z >= 0.0)
		return;

	// we are in front of first radius
	if (t1 < 0.0)
		return;
 
	state.hitPoint = state.ray.origin + t1 * state.ray.direction;
	float3 newDirection = glassSample(state.ray.direction, normalize(state.hitPoint - center1), true);
	state.ray = Ray::create(state.hitPoint, newDirection);
	
	// we are inside lens
	raySphereIntersection(state, center2, radius2, t1, t2);
	
	state.baryCoord = state.ray.origin + t2 * state.ray.direction;
	newDirection = glassSample(state.ray.direction, normalize(center2 - state.baryCoord), false);
	state.ray = Ray::create(state.baryCoord, newDirection);
}

void sphereBiconcave(inout State state, in float3 center1, in float3 center2, float radius1, float radius2, float3 minbox, float3 maxbox)
{
	float t1, t2;
	
	if (!rayAABBIntersection(state.ray, minbox, maxbox, t1, t2))
		return;

	state.glassHits++;
	if (state.ray.direction.z >= 0.0)
		return;
	
	raySphereIntersection(state, center1, radius1, t1, t2);
	
	// in front of first ball
	if (t2 < 0.0)
		return;	
	
	state.hitPoint = state.ray.origin + t2 * state.ray.direction;
	float3 newDirection = glassSample(state.ray.direction, normalize(center1 - state.hitPoint), true);
	state.ray = Ray::create(state.hitPoint, newDirection);
	
	// inside glass
	raySphereIntersection(state, center2, radius2, t1, t2);
		
	state.baryCoord = state.ray.origin + t1 * state.ray.direction;
	newDirection = glassSample(state.ray.direction, normalize(state.baryCoord - center2), false);
	state.ray = Ray::create(state.baryCoord, newDirection);
}

void spherePlanoConvex(inout State state, in float3 center1, float radius1, float3 minbox, float3 maxbox)
{
	float t1, t2;
	
	// intersect with box
	if (!rayAABBIntersection(state.ray, minbox, maxbox, t1, t2))
		return;

	// disable backward +z look
	float t3, t4;
	if (raySphereIntersection(state, center1, radius1, t3, t4))
		state.glassHits++;

	if (state.ray.direction.z >= 0.0)
		return;
	
	state.hitPoint = state.ray.origin + t1 * state.ray.direction;
	float3 newDirection = glassSample(state.ray.direction, float3(0, 0, 1), true);
	state.ray = Ray::create(state.hitPoint, newDirection);
	
	// inside glass
	raySphereIntersection(state, center1, radius1, t1, t2);
	
	state.baryCoord = state.ray.origin + t2 * state.ray.direction;
	newDirection = glassSample(state.ray.direction, normalize(center1 - state.baryCoord), false);
	state.ray = Ray::create(state.baryCoord, newDirection);
}

//////////////////////////////////

void traceLens(in Ray r, inout float4 outColor)
{
	State state; // todo this is shit
	state.ray = r;
	state.hitPoint = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	state.baryCoord = float3(0, 0, 0);
	state.glassHits = 0;

	float4 color = 0.7; // color of the lens
	float dummy;

	if (rayAABBIntersection(state.ray, lensMinBox, lensMaxBox, dummy, dummy))
	{
		for (uint i = 0; i < lensCount; ++i)
		{
			uint type = lens[i].type;

			if (type == 0) // biconcave
				sphereBiconcave(state, lens[i].center1, lens[i].center2, lens[i].radius1, lens[i].radius2, lens[i].minbox, lens[i].maxbox);
			else if (type == 1) // biconvex
				sphereBiconvex(state, lens[i].center1, lens[i].center2, lens[i].radius1, lens[i].radius2, lens[i].minbox, lens[i].maxbox);
			else if (type == 2) // planoconvex
				spherePlanoConvex(state, lens[i].center1, lens[i].radius1, lens[i].minbox, lens[i].maxbox);
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
	
	outColor += color * pow(0.8, state.glassHits); // TODO fix, based on real world
}

void traceChromaticLens(in Ray r, inout float4 outColor)
{
	float4 chromaticColor = 0.7;
	State state; // todo this is shit

	for (index = 0; index < 3; ++index)
	{
		state.ray = r;
		state.hitPoint = float3(FLT_MAX, FLT_MAX, FLT_MAX);
		state.baryCoord = float3(0, 0, 0);
		state.glassHits = 0;

		float4 color = 0.7; // color of the lens
		float dummy;

		if (rayAABBIntersection(state.ray, lensMinBox, lensMaxBox, dummy, dummy))
		{
			for (uint i = 0; i < lensCount; ++i)
			{
				uint type = lens[i].type;

				if (type == 0) // biconcave
					sphereBiconcave(state, lens[i].center1, lens[i].center2, lens[i].radius1, lens[i].radius2, lens[i].minbox, lens[i].maxbox);
				else if (type == 1) // biconvex
					sphereBiconvex(state, lens[i].center1, lens[i].center2, lens[i].radius1, lens[i].radius2, lens[i].minbox, lens[i].maxbox);
				else if (type == 2) // planoconvex
					spherePlanoConvex(state, lens[i].center1, lens[i].radius1, lens[i].minbox, lens[i].maxbox);
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

		if (index == 0)	chromaticColor.x = color.x;
		else if (index == 1) chromaticColor.y = color.y;
		else chromaticColor.z = color.z;
	}

	outColor += chromaticColor * pow(0.8, state.glassHits); // TODO fix, based on real world
}

void traceRays(in Ray r, inout float4 outColor)
{
	float dummy;
	if (rayAABBIntersection(r, raysMin, raysMax, dummy, dummy))
	{
		for (uint rc = 0; rc < raysCount; ++rc)
		{
			for (uint i = offsets[rc] + 1; i < offsets[rc + 1]; ++i)
			{
				if (rayCapsuleIntersection(r, points[i - 1], points[i], lineWidth))
					outColor = (outColor + float4(1, 0, 0, 0)) / 2;
			}
		}
	}
}

void createRayPoints()
{
	offsets[0] = 0;
	uint counter = 0;

	raysMin = FLT_MAX;
	raysMax = -FLT_MAX;

	for (uint rc = 0; rc < raysCount; ++rc)
	{
		addPoint(counter, rays[rc].origin, lineWidth);

		State state;
		state.ray = rays[rc];
		state.baryCoord = 0;
		state.glassHits = 0;

		float4 color = 0.7;

		for (uint i = 0; i < lensCount; ++i)
		{
			uint type = lens[i].type;
			state.hitPoint = FLT_MAX;

			if (type == 0) // biconcave
				sphereBiconcave(state, lens[i].center1, lens[i].center2, lens[i].radius1, lens[i].radius2, lens[i].minbox, lens[i].maxbox);
			else if (type == 1) // biconvex
				sphereBiconvex(state, lens[i].center1, lens[i].center2, lens[i].radius1, lens[i].radius2, lens[i].minbox, lens[i].maxbox);
			else if (type == 2) // planoconvex
				spherePlanoConvex(state, lens[i].center1, lens[i].radius1, lens[i].minbox, lens[i].maxbox);

			if (state.hitPoint.z < FLT_MAX)
			{
				addPoint(counter, state.hitPoint, lineWidth);
				addPoint(counter, state.baryCoord, lineWidth);
			}
		}

		float lastHit = FLT_MAX;
		for (uint i = 0; i < nTrianglesPlane; i++)
		{
			uint3 idx = planeIndices[i];

			if (rayTriangleIntersection(state, planeVertices[idx.x].xyz, planeVertices[idx.y].xyz, planeVertices[idx.z].xyz, lastHit))
				addPoint(counter, state.hitPoint, lineWidth);
		}

		if (lastHit == FLT_MAX)
			addPoint(counter, state.ray.direction * 100000, lineWidth);

		offsets[rc + 1] = counter;
	}
}

[numthreads(threadCountX, threadCountY, 1)]
void main(uint3 gid : SV_DispatchThreadID, uint tid : SV_GroupIndex)
{
	if (tid == 0)
		createRayPoints();
	
	AllMemoryBarrierWithGroupSync();

	float4 outColor = 0;
	Ray ray = getPrimaryRay(gid.xy);
	
	if (chromaticAberration)
		traceChromaticLens(ray, outColor);
	else 
		traceLens(ray, outColor);

	traceRays(ray, outColor);
		
	output[gid.xy] = outColor;
}