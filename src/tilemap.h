//
// Created by bzl on 21.06.2024.
//

#ifndef TILEMAP_H
#define TILEMAP_H

#include "global.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace BzlGame
{
struct Tile
{
	u16 type	{1};
	u8  assetID	{0};
};

struct Layer
{
	Vector<Tile> tileVec{{1, 0}	};
	bool isVisible{	true };
};

struct TileSet
{
	SharedPtr<Texture> texture;
	u16 height{};
	u16 width{};
};

struct Map
{
	Vector<Layer>	layer			{LAYER_NUMB_DEFAULT};
	//Vector<TileSet>	tileSets		{1};
	Vector<std::string> tileSetPaths{{BasePath"asset/tSet1.png"}	};

	u32 rows		= MAP_ROWS;
	u32 cols		= MAP_COLS;
	u16 tileSize	= TILE_SIZE;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tile, type, assetID)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Layer, tileVec, isVisible);
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Map, rows, cols, tileSize, TileSetPaths, layer)

}


#endif //TILEMAP_H
