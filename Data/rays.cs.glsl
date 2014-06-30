#version 430

layout(RGBA32F) uniform writeonly image2D outImage;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	vec3 color = vec3(0.3, 1.0, 0.3);
	imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0f));
}