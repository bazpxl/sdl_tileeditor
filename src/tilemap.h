#ifndef TILEMAP_H
#define TILEMAP_H

#include "global.h"
#include "nlohmann/json.hpp"

namespace BzlGame
{

struct Tile
{
	u16 type	{0};
	u8  asset_id	{0};
};

struct TileSet
{
	SharedPtr<Texture> texture;
	Point size;
	string path;
};

/// generate with these macro inline serialization function, to and from JSON for Tile
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tile, type, asset_id)
inline int pointToInt(const Point r, const int xMax ) { return r.x + r.y * xMax; }
inline Point intToPoint(const int s, const int xMax ) { return { s % xMax, s / xMax }; }

class Map
{
private:
	static constexpr u16 MapRows		= 50;
	static constexpr u16 MapCols		= 50;
	static constexpr u16 TileSize		= 32;

	Vector<TileSet> tilesets_;
	Vector<Vector<Tile>> tiles_ {LayerNumb-1, Vector<Tile>(MapRows*MapCols,{EmptyTileVal,0})};

	u16 rows_{};
	u16 cols_{};
	u16 tilesize_{};
	u8  layer_number_{};
public:
	// ctor empty map without tilesets
	explicit Map(const u16 rows = MapRows,		const u16 cols	= MapCols,
				 const u16 tilesize = TileSize, const u8  layer_numb = LayerNumb)
		: rows_(rows), cols_(cols),
		  tilesize_(tilesize), layer_number_(layer_numb)
	{
		tiles_ =
		{
			static_cast <size_t>(layer_numb),
			Vector<Tile>(rows*cols,
			{EmptyTileVal,0})
		};
	}

	// ctor needs params path & renderer, other params are optional
	// create map and read data from file
	explicit Map( const string & path, Renderer* render)
		: rows_(MapRows), cols_(MapCols),
		  tilesize_(TileSize), layer_number_(LayerNumb)
	{
		tiles_ =	{static_cast <size_t>(layer_number_)-1, Vector<Tile>(rows_*cols_,{EmptyTileVal,0})};
		ReadJson(path, render);
	}

	void ReadJson(const string & path, Renderer * render);
	void WriteJson(const string & path);

	void AddTileset(const SharedPtr<Texture> & texture,const Point size, const string& path)
	{
		tilesets_.push_back({texture, size, path});
		layer_number_++;
	}

	void RemoveTileset(const int id)
	{
		assert(tilesets_.size() > id);
		tilesets_.erase(tilesets_.begin() + id);
		layer_number_ = static_cast<u8>(tilesets_.size());
	}

	void setTile(const u8 layer, const u16 id, const Tile tile)
	{
		DebugOnly(
			assert(tiles_.size() > layer);
			assert(tiles_[layer].size() > id);
			)
		tiles_[layer][id] = tile;
	}


	[[nodiscard]] Vector<Tile>			&			getLayer(const int index)			 {	return tiles_.at(index);	}
	[[nodiscard]] Vector<Vector<Tile>>	&			getTileVec()							 {	return tiles_;				}
	[[nodiscard]] Vector<TileSet>		&			getTilesets()						 {	return tilesets_;			}
	[[nodiscard]] TileSet				&			getTileset(const int id)			 {	return tilesets_[id];		}
	[[nodiscard]] Point								getTilesetSize(const int id)	const{	return tilesets_[id].size;	}
	[[nodiscard]] bool								isNoTileSet()					const{	return tilesets_.empty();	}
	[[nodiscard]] u16								rows()							const{	return rows_;				}
	[[nodiscard]] u16								cols()							const{	return cols_;				}
	[[nodiscard]] u16								tilesize()						const{	return tilesize_;			}
	[[nodiscard]] u8								layer_number()					const{	return layer_number_;			}

};


//--------------------------------------------------

}


#endif //TILEMAP_H
