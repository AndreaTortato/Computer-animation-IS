#version 450 core

in vec3 norm;
in vec3 fragPos;
in vec2 uv;

uniform vec3 light;  
uniform sampler2D tex0;

out vec4 FragColor;

void main() {
	vec4 diffuseColor = texture(tex0, uv);

	vec3 n = normalize(norm);
	vec3 l = normalize(light);
	float diffuseIntensity = clamp(dot(n, l) , 0, 1);
	float ambient = 0.1;
	diffuseColor.rgb = clamp((diffuseColor.rgb * diffuseIntensity) + ambient , 0, 1);
	FragColor =  vec4(diffuseColor, 1.0);
}