#include "Terrain.h"

Terrain::Terrain()
{
	textureID = 0;
}

void Terrain::initialize(GLuint texture1, GLuint texture2)
{
	textureID = texture2;
	Chunk chunk, chunk2, chunk3;
	chunk.initialize(texture1, 100.0f, 0.5f, 100.0f);
	chunks.push_back(chunk);
	chunk2.initialize(texture1, 100.0f, 0.5f, -100.0f);
	chunks.push_back(chunk2);
	chunk3.initialize(textureID, 100.0f, 0.5f, -300.0f);
	chunks.push_back(chunk3);
}

void Terrain::addChunk()
{
	Chunk chunk;
	chunk.initialize(textureID, chunks.back().getX(), chunks.back().getY(), chunks.back().getZ() - 200.0f);
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

void Terrain::setTexture(GLuint texture)
{
	textureID = texture;
	chunks.back().setTextureID(texture);
}

std::vector<Chunk> Terrain::getChunks()
{
	return chunks;
}
