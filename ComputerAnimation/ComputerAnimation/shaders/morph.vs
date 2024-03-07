#version 330 core
#define MORPHTARGETS_COUNT 50

uniform mat4 model;
uniform mat4 view_projection;
uniform mat4 pose[100];
uniform mat4 invBindPose[100];

uniform sampler2D morphTargetsTexture;
uniform ivec2 morphTargetsTextureSize;

uniform int numMorphTargets;
uniform float morphTargetInfluences[ MORPHTARGETS_COUNT ];
uniform int numVertices;

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec4 weights;
in ivec4 joints;

out vec3 norm;
out vec3 fragPos;
out vec2 uv;

// [CA] To do: Create a function to get the vertex position offset given the index of the vertex and the index of the morph target
vec4 getMorph( const int vertexIndex, const int morphTargetIndex ) 
{
	ivec2 texCoord = ivec2(vertexIndex, morphTargetIndex);

	// used to fetch texels from a texture
	vec4 offset = texelFetch(morphTargetsTexture, texCoord, 0); 
	return offset;
}

void main() 
{

	vec3 transformed = vec3( position );

	// [CA] To do: For each morph target, accumulate the offset position taking into account its influence
	for (int i = 0; i < numMorphTargets; ++i) 
	{
        vec4 offset = getMorph(gl_VertexID, i);
        transformed += morphTargetInfluences[i] * offset.xyz;
    }

	// Compute skinning
	mat4 skin = (pose[joints.x] * invBindPose[joints.x]) * weights.x;
    skin += (pose[joints.y] * invBindPose[joints.y]) * weights.y;
    skin += (pose[joints.z] * invBindPose[joints.z]) * weights.z;
    skin += (pose[joints.w] * invBindPose[joints.w]) * weights.w;

	// Transform the final computed vertex position into clip space
    gl_Position = view_projection * model * skin * vec4(transformed,1.0);
    fragPos = vec3(model * skin * vec4(transformed, 1.0));
    norm = vec3(model * skin * vec4(normal, 0.0f));
    uv = texCoord;
   
}