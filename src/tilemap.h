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
	Vector<string> tileSetPaths{{BasePath"asset/tSet1.png"}	};

	u32 rows		= MAP_ROWS;
	u32 cols		= MAP_COLS;
	u16 tileSize	= TILE_SIZE;
};

/// generate with these macros inline conversion functions,
/// to and from JSON for Tile and Layer structs
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
		throw std::invalid_argument("Error: Could not load file: " + path );
	}
	file << std::setw(1) << dataJson << std::endl;
	file.close();
}


inline json serializeToJson(const Map& map)
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
