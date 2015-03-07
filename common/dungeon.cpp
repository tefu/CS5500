#include <cmath>
#include <time.h>
#include "dungeon.h"
#include "logger.h"

void Dungeon::makeDungeon(ChunkList& list)
{
  list.resize(DUNGEON_SIZE * DUNGEON_SIZE * DUNGEON_SIZE);
  for (int i = 0; i < DUNGEON_SIZE; i++)
  {
    for (int j = 0; j < DUNGEON_SIZE; j++)
    {
      for (int k = 0; k < DUNGEON_SIZE; k++)
      {
        list[index(i, j, k)] = new Chunk(i, j, k);
      }
    }
  }
  makeChunksActive(list);
  createRooms(list);
}

void Dungeon::makeChunksActive(ChunkList& chunkList)
{
  for (auto& c : chunkList)
  {
    c->activateAllBlocks();
  }
}

int Dungeon::index(int x, int y, int z)
{
  return x * DUNGEON_SIZE * DUNGEON_SIZE + y * DUNGEON_SIZE + z;
}

bool Dungeon::isBlockActive(const ChunkList& list, int x, int y, int z)
{
  if (x < 0 || dungeonBlockLength() <= x || y < 0 ||
      dungeonBlockLength() <= y || z < 0 || dungeonBlockLength() <= z)
  {
    return false;
  }

  auto chunk =
    list[index(x / DUNGEON_SIZE, y / DUNGEON_SIZE, z / DUNGEON_SIZE)];

  return chunk->isBlockActive(
    x % DUNGEON_SIZE, y % DUNGEON_SIZE, z % DUNGEON_SIZE);
}

void Dungeon::createRooms(ChunkList& list)
{
	// Entrance is at the top of our dungeon cube.

	std::vector<Chunk*> roomList;
	struct roomPair
	{
		Chunk* chunka;
		Chunk* chunkb;
	};
	std::vector<roomPair> connectList;
	roomPair pair;
	Chunk* closestChunk;
	//float closestDist;
	Chunk* currentChunk;
	srand(time(NULL));
	int j;
	unsigned int i;
	int roomcount;
	int x,y,z; //location
	// make entrance
	x = DUNGEON_SIZE / 2;
	y = DUNGEON_SIZE / 2;
	z = DUNGEON_SIZE -1;
	auto chunkrand = list[index( x , y , z)];
	createRoom(chunkrand);
	roomList.push_back(chunkrand);
	roomcount  = DUNGEON_SIZE * DUNGEON_SIZE * DUNGEON_SIZE * .05;

	// Create random rooms
	for(j = DUNGEON_SIZE-1; j >= 0; j--)
	{

		z = j; //floor j

		for(i = 0; i < (unsigned int)(roomcount/DUNGEON_SIZE); i++)
		{
			y = rand() % DUNGEON_SIZE;
			x = rand() % DUNGEON_SIZE;
			chunkrand = list[index(x, y, z)];
			roomList.push_back(chunkrand);
			createRoom(chunkrand);
			if(roomList.size() > 1)
			{
				closestChunk = roomList[roomList.size()-1];
				currentChunk = roomList[roomList.size()-2];
				pair.chunka = closestChunk;
				pair.chunkb = currentChunk;
				connectList.push_back(pair);
			}
		}



		// Till vector is empty, connect all rooms on this floor
		for(i = 0;i < connectList.size(); i++)
		{
			connectRoom(list, connectList[i].chunka,connectList[i].chunkb);
		}
		connectList.clear();

	}
}

void Dungeon::connectRoom(ChunkList& list, Chunk* chunk1, Chunk* chunk2)
{
  auto pos1 = chunkToBlockDistance(chunk1->getPosition());
  auto pos2 = chunkToBlockDistance(chunk2->getPosition());

  auto dist = distanceBetween(pos1, pos2);
  auto unit = -((pos1 - pos2) / dist);

  auto start = pos1;

  auto in_dungeon = [](Vector3 v)
  {
    return (0 < v.x && v.x < DUNGEON_SIZE * Chunk::CHUNK_SIZE - 1 && 0 < v.y &&
            v.y < DUNGEON_SIZE * Chunk::CHUNK_SIZE - 1 && 0 < v.x &&
            v.x < DUNGEON_SIZE * Chunk::CHUNK_SIZE - 1);
  };
  while (distanceBetween(start, pos2) > 1.0 && in_dungeon(start))
  {
    start += unit;
    list[index((static_cast<int>(start.x)) / DUNGEON_SIZE,
               (static_cast<int>(start.y)) / DUNGEON_SIZE,
               (static_cast<int>(start.z)) / DUNGEON_SIZE)]
        ->deactivateBlock((static_cast<int>(start.x)) % DUNGEON_SIZE,
                          (static_cast<int>(start.y)) % DUNGEON_SIZE,
                          (static_cast<int>(start.z)) % DUNGEON_SIZE);
  }
}

void Dungeon::createRoom(Chunk* chunk)
{
  // Rooms are just empty chunks for now.
  chunk->deactivateAllBlocks();
}

int Dungeon::dungeonBlockLength()
{
  return DUNGEON_SIZE * Chunk::CHUNK_SIZE;
}

void Dungeon::printDungeon(ChunkList& list)
{
  list.resize(DUNGEON_SIZE * DUNGEON_SIZE * DUNGEON_SIZE);
  for (int k = DUNGEON_SIZE - 1; k >= 0; k--)
  {
    std::cout << "-----" << k << "f-----------------------------------------"
              << std::endl;
    for (int j = DUNGEON_SIZE - 1; j >= 0; j--)
    {
      for (int i = 0; i < DUNGEON_SIZE; i++)
      {
        if (isChunkAllActive(list[index(i, j, k)]))
        {
          // Solid active chunk.
          std::cout << " . ";
        }
        else if (isChunkAnyActive(list[index(i, j, k)]))
        {
          // Path.
          std::cout << " o ";
        }
        else
        {
          // Room.
          std::cout << " R ";
        }
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

bool Dungeon::isChunkAllActive(Chunk* chunk)
{
  for (int i = 0; i < Chunk::CHUNK_SIZE; i++)
  {
    for (int j = 0; j < Chunk::CHUNK_SIZE; j++)
    {
      for (int k = 0; k < Chunk::CHUNK_SIZE; k++)
      {
        if (!(chunk->isBlockActive(i, j, k))) return false;
      }
    }
  }
  return true;
}

bool Dungeon::isChunkAnyActive(Chunk* chunk)
{
  for (int i = 0; i < Chunk::CHUNK_SIZE; i++)
  {
    for (int j = 0; j < Chunk::CHUNK_SIZE; j++)
    {
      for (int k = 0; k < Chunk::CHUNK_SIZE; k++)
      {
        if (chunk->isBlockActive(i, j, k)) return true;
      }
    }
  }
  return false;
}

float Dungeon::distanceBetween(Vector3 pos1, Vector3 pos2)
{
  return sqrt(pow(fabs(pos1.x - pos2.x), 2.0) +
              pow(fabs(pos1.y - pos2.y), 2.0) +
              pow(fabs(pos1.z - pos2.z), 2.0));
}

Vector3 Dungeon::chunkToBlockDistance(Vector3 incoming)
{
  Vector3 forYou;

  forYou.x = incoming.x * Chunk::CHUNK_SIZE;
  forYou.y = incoming.y * Chunk::CHUNK_SIZE;
  forYou.z = incoming.z * Chunk::CHUNK_SIZE;

  return forYou;
}
