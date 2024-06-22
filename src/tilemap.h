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
	//Vector<Tile> tileVec{{1, 0}	};
	Vector<Tile> tileVec{MAP_COLS * MAP_ROWS};
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


/// @brief write a JSON-type to a given path
/// @param dataJson const JSON reference to write from
/// @param path const char ptr for map filepath.
/// @return no returned val
inline void writeJson(const json& dataJson, const char* path)
{
	std::ofstream file{path};
	if(!file)[[unlikely]]
	{
		throw std::invalid_argument("Error: Could not load file: asset/map.json");
	}else[[likely]]
	{
		file << std::setw(1) << dataJson << std::endl;
		file.close();
	}
}

/// @brief serializing the "struct Map" to JSON file format
/// @param map const ref to struct Map
/// @return returns the serialized JSON type
inline nlohmann::json serializeToJson(const Map& map)
{
	return json{
		{"rows", map.rows},
		{"cols", map.cols},
		{"tileSize", map.tileSize},
		{ "layer", map.layer},
		{"tileSetPaths", map.tileSetPaths}
	};
}

}


#endif //TILEMAP_H
