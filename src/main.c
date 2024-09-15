#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "shader.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#define CELL_SIZE 2
#define CELL_ROWS SCR_HEIGHT/CELL_SIZE
#define CELL_COLS SCR_WIDTH/CELL_SIZE

int delay = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
	} else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		delay += 50;
	} else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		if (delay > 0) {
			delay -= 50;
		}
	}
}

int get_live_neighbor_count(bool map[CELL_ROWS][CELL_COLS], int row, int col) {
	int i, j, count = 0;

	for (i = row-1;i<=row+1;i++) {
		for (j = col-1;j<=col+1;j++) {
			if ((i == row && j == col) || (i < 0 || j < 0) || (i >= CELL_ROWS || j >= CELL_COLS)) {
				continue;
			}

			if (map[i][j]) {
				count++;
			}
		}
	}

	return count;
}

int main(int argc, char **argv) {
	if (!glfwInit()) {
		fprintf(stderr, "failed to initialize glfw");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Conway's game of life", NULL, NULL);
	if (!window) {
		fprintf(stderr, "failed to create window");
		return -1;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	Shader shader_pg;
	shader_init(&shader_pg, "assets/shader.vert", "assets/shader.frag");

	float vertices[] = {
		0.0f, 0.0f,
		CELL_SIZE, 0.0f, 
		CELL_SIZE, CELL_SIZE, 
		0.0f, CELL_SIZE, 
	};
	
	unsigned int indices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	bool map_a[CELL_ROWS][CELL_COLS], map_b[CELL_ROWS][CELL_COLS];
	bool (*map_active)[CELL_ROWS][CELL_COLS] = &map_a;
	bool (*map_inactive)[CELL_ROWS][CELL_COLS] = &map_b;

	srand(time(NULL));

	for(int i = 0; i < CELL_ROWS; i++) {
		for(int j = 0; j < CELL_COLS; j++) {
			(*map_active)[i][j] = rand() & 1;
		}
	}

	glUseProgram(shader_pg);

	mat4 projection, model;
	glm_ortho(0.0f, SCR_WIDTH, SCR_HEIGHT, 0.0f, -1.0f, 1.0f, projection);
	shader_set_mat4(shader_pg, "projection", projection);

	while(!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for(int i = 0; i < CELL_ROWS; i++) {
			for(int j = 0; j < CELL_COLS; j++) {
				int count = get_live_neighbor_count(*map_active, i, j);
				if (count < 2 || count > 3) {
					(*map_inactive)[i][j] = false;
				} else if (count == 3 || ((*map_active)[i][j] && count == 2)) {
					(*map_inactive)[i][j] = true;
				} else {
					(*map_inactive)[i][j] = false;
				}
			}
		}

		map_active = (bool (*)[CELL_ROWS][CELL_COLS])((uintptr_t)map_active ^ (uintptr_t)map_inactive);
		map_inactive = (bool (*)[CELL_ROWS][CELL_COLS])((uintptr_t)map_active ^ (uintptr_t)map_inactive);
		map_active = (bool (*)[CELL_ROWS][CELL_COLS])((uintptr_t)map_active ^ (uintptr_t)map_inactive);

		glBindVertexArray(vao);

		for(int i = 0; i < CELL_ROWS; i++) {
			for(int j = 0; j < CELL_COLS; j++) {
				glm_mat4_identity(model);
				glm_translate(model, (vec3){j * CELL_SIZE, i * CELL_SIZE, 0.0f});

				shader_set_mat4(shader_pg, "model", model);
				shader_set_bool(shader_pg, "alive", (*map_active)[i][j]);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		usleep(delay * 1000);
	}

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(shader_pg);
	glfwTerminate();

	return 0;
}

