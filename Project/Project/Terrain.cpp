#include "Terrain.h"

Terrain::Terrain()
{
	textureID = 0;
}

void Terrain::initialize(GLuint texture)
{
	textureID = texture;
	Chunk chunk, chunk2;
	chunk.initialize(textureID, 100.0f, 3.5f, -100.0f);
	chunks.push_back(chunk);
	chunk2.initialize(textureID, 100.0f, 3.5f, -300.0f);
	chunks.push_back(chunk2);
}

void Terrain::addChunk()
{
	Chunk chunk;
	chunk.initialize(chunks.back().getTextureID(), chunks.back().getX(), chunks.back().getY(), chunks.back().getZ() - 200.0f);
	chunks.push_back(chunk);

	deleteChunk();
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
