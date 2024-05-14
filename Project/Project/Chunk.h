#pragma once

#include <GL/glew.h>
#include <vector>

class Chunk {
private:
	GLuint VAO, VBO;
	GLsizei indexCount;
	GLuint textureID;
	float position[3];

public:
	Chunk();
	void initialize(GLuint texture, float xPos, float yPos, float zPos);

	void render();
	void clean();

	float getX();
	float getY();
	float getZ();

	GLuint getTextureID();
};
