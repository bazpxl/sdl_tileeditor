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
constexpr u16 MapRows		= 50;
constexpr u16 MapCols		= 50;

constexpr u8 LayerNumb		=  3;
constexpr u8 CameraSpeed	=  4;


struct Tile
{
	u16 type	{0};
	u8  assetID	{0};
};

struct MapHeader
{
	Vector<string> tileSetPaths;
	u16 rows			= MapRows;
	u16 cols			= MapCols;
	u16 tileSize		= TileSize;
	u8  layerNumb		= LayerNumb;
};

struct MapData
{
	Vector<SharedPtr<Texture>> tileSets;
	Vector<Vector<Tile>> tiles {LayerNumb, Vector<Tile>(MapRows*MapCols,{0,0})};
};

inline int pointToInt( Point r, int xMax ) { return r.x + r.y * xMax; }
inline Point intToPoint( int s, int xMax ) { return Point { s % xMax, s / xMax }; }

//------------------------------------------------------------------------
//------------------------------------------------------------------------

/// generate with these macro inline serialization function, to and from JSON for Tile
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tile, type, assetID)

inline void readJson(const string& path , MapHeader& header, MapData& data, Renderer* render)
{
	std::ifstream file(path);
	if(!file.is_open())[[unlikely]]
	{
		throw std::invalid_argument("Could'nt open file: " + path );
	}
	json j = json::parse(file);

	header.tileSetPaths =	j["tileSetPaths"].template get<Vector<string>>();
	header.rows			=	j["rows"].template get<u16>();
	header.cols			=	j["cols"].template get<u16>();
	header.tileSize		=	j["tileSize"].template get<u8>();
	header.layerNumb	=	j["layerNumb"].template get<u8>();

	data.tiles			=	j["tiles"].template get<Vector<Vector<Tile>>>();

	for(auto & vec : header.tileSetPaths)
	{
		string tmpstr = BasePath;
		tmpstr.append(vec);
		DebugOnly(
			print("{}",tmpstr);
		)
		data.tileSets.push_back(CreateSharedTexture(render, tmpstr.c_str()));

	}
	file.close();
}

inline void writeJson(const string& path , MapHeader& header, MapData& data)
{
	// serialize to json
	json dataJson{
			{"tileSetPaths", header.tileSetPaths},
			{"rows", header.rows},
			{"cols", header.cols},
			{"tileSize", header.tileSize},
			{ "layerNumb", header.layerNumb},
			{"tiles", data.tiles}
	};

	std::ofstream file{path};
	if(!file)[[unlikely]]
	{
		throw std::invalid_argument("Error: Could not load file: " + path );
	}
	file <<  dataJson << std::endl;
	file.close();
}

}


#endif //TILEMAP_H
