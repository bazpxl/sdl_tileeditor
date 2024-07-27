#ifndef TILEMAP_H
#define TILEMAP_H

#include "global.h"
#include "nlohmann/json.hpp"


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

struct TileSet
{
	SharedPtr<Texture> texture;
	Point size;
	string path;

};

/// generate with these macro inline serialization function, to and from JSON for Tile
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tile, type, asset_id)

inline int pointToInt( Point r, int xMax ) { return r.x + r.y * xMax; }
inline Point intToPoint( int s, int xMax ) { return Point { s % xMax, s / xMax }; }


class Map
{
private:
	Vector<TileSet> tilesets_;
	Vector<Vector<Tile>> tiles_ {LayerNumb-1, Vector<Tile>(MapRows*MapCols,{EmptyTileVal,0})};

	u16 rows_{};
	u16 cols_{};
	u16 tilesize_{};
	u8  layer_numb_{};
public:

	void ReadJson(const string & path, Renderer * render);
	void WriteJson(const string & path);


	explicit Map(const u16 rows = MapRows, const u16 cols = MapCols, const u16 tilesize = TileSize, const u8 layer_numb = LayerNumb)
		: rows_(rows),
		  cols_(cols),
		  tilesize_(tilesize),
		  layer_numb_(layer_numb)
	{
		Vector<Vector<Tile>> tiles_ {static_cast <size_t>(layer_numb)-1, Vector<Tile>(rows*cols,{EmptyTileVal,0})};
	}

	void AddTileset(const SharedPtr<Texture> & texture,const Point size, const string& path)
	{
		tilesets_.push_back({texture, size, path});
	}

	void RemoveTileset(const int id)
	{
		assert(tilesets_.size() > id);
		tilesets_.erase(tilesets_.begin() + id);
	}

	void setTile(const u8 layer, const u16 id, const Tile tile)
	{
		DebugOnly(
			assert(tiles_.size() > layer);
			assert(tiles_[layer].size() > id);
			)
		tiles_[layer][id] = tile;
	}


	[[nodiscard]]  Vector<Tile>			&			getLayer(const int index)			 {	return tiles_.at(index);	}
	[[nodiscard]]  Vector<Vector<Tile>>	&			get_tiles()							 {	return tiles_;				}
	[[nodiscard]] Vector<TileSet>		&			getTilesets()						 {	return tilesets_;			}
	[[nodiscard]] TileSet				&			getTileset(const int id)			 {	return tilesets_[id];		}
	[[nodiscard]] Point								getTilesetSize(const int id)	const{	return tilesets_[id].size;	}
	[[nodiscard]] bool								isNoTileSet()					const{	return tilesets_.empty();	}
	[[nodiscard]] u16								rows()							const{	return rows_;				}
	[[nodiscard]] u16								cols()							const{	return cols_;				}
	[[nodiscard]] u16								tilesize()						const{	return tilesize_;			}
	[[nodiscard]] u8								layer_numb()					const{	return layer_numb_;			}

};


//--------------------------------------------------

}


#endif //TILEMAP_H
