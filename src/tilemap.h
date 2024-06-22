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


inline Map jsonToMap(const json& j)
{
	return {
		j["layer"].template get<Vector<Layer>>(),
		j["tileSetPaths"].template get<Vector<string>>(),
		j["rows"].template get<u32>(),
		j["cols"].template get<u32>(),
		j["tileSize"].template get<u16>()
		};
}

inline void writeJson(const json& dataJson, const string& path = BasePath"asset/map.json")
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


inline json readJson(const string& path = BasePath"asset/map.json")
{
	std::ifstream file(path);
	if(!file.is_open())[[unlikely]]
	{
		throw std::invalid_argument("Could'nt open file: " + path );
	}
	json jsTemp = json::parse(file);

	//DebugOnly(	println("{}", jsTemp.dump(0));	)
	return jsTemp;
}

}


#endif //TILEMAP_H
