#include "Chunk.h"

Chunk::Chunk()
{
}

void Chunk::initialize(GLuint texture, float xPos, float yPos, float zPos)
{
	GLfloat vertices[] = {
		// First triangle
		xPos, -yPos, zPos, 0.0f, 1.0f, 0.0f, 100.0f, 0.0f, //A - E
		-xPos, -yPos, zPos, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //B - F
		-xPos, -yPos, zPos + 200.0f, 0.0f, 1.0f, 0.0f, 0.0f, 100.0f, //C - G

		// Second triangle
		xPos, -yPos, zPos, 0.0f, 1.0f, 0.0f, 100.0f, 0.0f, //A - E
		-xPos, -yPos, zPos + 200.0f, 0.0f, 1.0f, 0.0f, 0.0f, 100.0f, //C - G
		xPos, -yPos, zPos + 200.0f, 0.0f, 1.0f, 0.0f, 100.0f, 100.0f // D - H
	};

	position[0] = xPos;
	position[1] = yPos;
	position[2] = zPos;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	indexCount = 6;

	// Store texture ID
	textureID = texture;
}

void Chunk::render()
{
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, indexCount);
	glBindVertexArray(0);
}

void Chunk::clean()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

float Chunk::getX() {
	return position[0];
}

float Chunk::getY() {
	return position[1];
}

float Chunk::getZ() {
	return position[2];
}

GLuint Chunk::getTextureID()
{
	return textureID;
}





