#include "Terrain.h"

Terrain::Terrain()
{
	textureID = 0;
}

void Terrain::initialize(GLuint texture)
{
	textureID = texture;
	Chunk chunk;
	chunk.initialize(textureID, 100.0f, 3.5f, -100.0f);
	chunks.push_back(chunk);
}

void Terrain::addChunk()
{
	Chunk chunk;
	chunk.initialize(chunks.back().getTextureID(), chunks.back().getXPosition(), chunks.back().getYPosition() + 2.f, chunks.back().getZPosition() - 200.0f);
	chunks.push_back(chunk);
}

void Terrain::render()
{
	for (auto& chunk : chunks) {
		chunk.render();
	}
}

void Terrain::clean()
{
	for (auto& chunk : chunks) {
		chunk.clean();
	}
}

void Terrain::deleteChunk()
{
	chunks.front().clean();
	chunks.erase(chunks.begin());
}

std::vector<Chunk> Terrain::getChunks()
{
	return chunks;
}
