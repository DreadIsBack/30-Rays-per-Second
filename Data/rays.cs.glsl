#version 430

#define VIEW_DISTANCE 100000.0f

struct Ray
{
	vec3 origin;
	vec3 dir;
};

struct IntersactionResult
{
	int objIdx;
	vec3 point;
	float distance;
};

struct Quad
{
	vec4 a, b, c, d;
	vec4 center;

	// Material
	vec4 diffuse;

	// Required for intersection test. In 'xyz' holded normal vector, and in 'w' holded dot products
	vec4 N, N1, N2, N3, N4;
};

layout(std430, binding=0) readonly buffer QuadBuffer
{
	Quad quads[];
};
uniform int gNumQuads;

uniform vec3 gCameraPos;
uniform mat3 gCameraRot;
layout(RGBA32F) uniform writeonly image2D outImage;


bool IntersectRayQuad(Ray r, int q, float maxDistance, out IntersactionResult result)
{
	result.objIdx = q;
	float NdotR = -dot(quads[q].N.xyz, r.dir);
	//if (NdotR > 0.0f) // || (Refraction > 0.0f && NdotR < 0.0f))
	{
		result.distance = (dot(quads[q].N.xyz, r.origin) + quads[q].N.w) / NdotR;
		if (result.distance >= 0.0f && result.distance < maxDistance)
		{
			result.point = r.dir * result.distance + r.origin;

			if (dot(quads[q].N1.xyz, result.point) + quads[q].N1.w < 0.0f) return false;
			if (dot(quads[q].N2.xyz, result.point) + quads[q].N2.w < 0.0f) return false;
			if (dot(quads[q].N3.xyz, result.point) + quads[q].N3.w < 0.0f) return false;
			if (dot(quads[q].N4.xyz, result.point) + quads[q].N4.w < 0.0f) return false;

			return true;
		}
	}

	return false;
}


layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	// Compute projection
	const float cWidth = 800.0; // TODO: hardocde is bad
	const float cHeight = 600.0; // TODO: hardocde is bad
	float viewRayX = 1.0f - ((gl_GlobalInvocationID.x * 2) / cWidth);
	float viewRayY = -(1.0f - ((gl_GlobalInvocationID.y * 2) / cHeight)); // invert Y, becouse in OpenGL positive Y in bottom side
	Ray viewRay = Ray(gCameraPos, normalize(gCameraRot * vec3(viewRayX, viewRayY, -1))); // TODO: apply rotation from CameraDir, not from matrix. If this really can make.


	// Ray Casting
	float distance = VIEW_DISTANCE;
	IntersactionResult tempResult = IntersactionResult(-1,vec3(0,0,0),0);
	IntersactionResult result = IntersactionResult(-1,vec3(0,0,0),0);
	for (int i = 0; i < gNumQuads; i++)
	{
		if (IntersectRayQuad(viewRay, i, distance, tempResult))
		{
			distance = tempResult.distance;
			result = tempResult;
		}
	}


	vec3 color = vec3(0.7f, 0.7f, 1.0f); // background color
	if (result.objIdx != -1) // have intersaction
	{
		// Lighting from camera
		float NdotCD = dot(quads[result.objIdx].N.xyz, -viewRay.dir);
		color = quads[result.objIdx].diffuse.xyz * abs(NdotCD);
	}
	else // no intersaction
	{
		// Fake sky
		if (viewRay.dir.y > 0.0f)
		{
			vec3 skyColorZenith = vec3(0.3f, 0.3f, 1.0f);
			vec3 skyColorHorizont = vec3(0.7f, 0.7f, 1.0f);
			vec3 sunColor = vec3(1.0f, 0.7f, 0.7f);
			vec3 zenith = vec3(0,1,0);
			vec3 sunDir = normalize(vec3(0,0.5f,-1)); // TODO: to uniform
			color = mix(skyColorHorizont, skyColorZenith, max(0, dot(viewRay.dir, zenith))); // compute sky color
			color += sunColor * pow(max(0, dot(viewRay.dir, sunDir)), 80); // apply sun, 80 - sun size
		}
	}

	imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0f));
}