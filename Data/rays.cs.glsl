#version 430

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

layout(RGBA32F) uniform writeonly image2D outImage;


layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	vec3 color = vec3(0.3, 1.0, 0.3);
	imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0f));
}