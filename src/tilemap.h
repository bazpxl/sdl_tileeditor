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
constexpr u16 TileSize		= 32;

constexpr u8 LayerNumb		=  3;
constexpr u8 CameraSpeed	=  4;
constexpr u8 EmptyTileVal	= 255;

struct Tile
{
	u16 type	{0};
	u8  asset_id	{0};
};

struct MapHeader
{
	Vector<string> asset_paths;
	u16 rows			= MapRows;
	u16 cols			= MapCols;
	u16 tilesize		= TileSize;
	u8  layer_numb		= LayerNumb;
};

struct MapData
{
	Vector<SharedPtr<Texture>> tileSets;
	Vector<Vector<Tile>> tiles {LayerNumb, Vector<Tile>(MapRows*MapCols,{EmptyTileVal,0})};
};

//------------------------------------------------------------------------

inline int pointToInt( Point r, int xMax ) { return r.x + r.y * xMax; }
inline Point intToPoint( int s, int xMax ) { return Point { s % xMax, s / xMax }; }

/// generate with these macro inline serialization function, to and from JSON for Tile
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tile, type, asset_id)

}


#endif //TILEMAP_H
