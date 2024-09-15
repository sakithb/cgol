#version 460 core

out vec4 FragColor;

uniform bool alive;

void main() {
	if (alive) {
		FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else {
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
