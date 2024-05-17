#pragma once

#include "Chunk.h"

class Terrain {
public:
	Terrain();
	void initialize(GLuint texture1, GLuint texture2);

	void addChunk();
	void deleteChunk();

	void render();
	void clean();

	void setTexture(GLuint texture);
	std::vector<Chunk> getChunks();

private:
	GLuint textureID;
	std::vector<Chunk> chunks;
};