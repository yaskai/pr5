#version 330

in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec3 fragPosition;
in vec3 fragWorldPos;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float overbright;
uniform int draw_mode;

out vec4 finalColor;

void main() {
	vec4 diffuse = texture(texture0, fragTexCoord);
	vec4 light = texture(texture1, fragTexCoord2);

	finalColor = (light * overbright); 
	if(draw_mode == 0) finalColor *= diffuse;

	finalColor.a = diffuse.a;
}
