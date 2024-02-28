#version 450 core

in vec3 norm;
in vec3 fragPos;
in vec2 uv;

uniform vec3 light;  

out vec4 FragColor;

void main() {
	vec3 n = normalize(norm);
	vec3 l = normalize(light);
	float diffuseIntensity = clamp(dot(n, l) , 0, 1);
	FragColor =  vec4(vec3(diffuseIntensity), 1.0);
}