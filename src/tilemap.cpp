
#include "tilemap.h"
#include <filesystem>
using namespace BzlGame;

void Map::setTile(const u8 layer, const u16 id, const Tile tile) {
	DebugOnly(
		assert(tiles_.size() > layer);
		assert(tiles_[layer].size() > id);
	)
	tiles_[layer][id] = tile;
}

void Map::AddLayer() {
	tiles_.push_back(Vector<Tile>(rows_*cols_, {EmptyTileVal, 0}));
	layer_number_++;
}

void Map::RemoveLayer(const int id) {
	if(id < tiles_.size())
	{
		tiles_.erase(tiles_.begin() + id);
		layer_number_--;
	}
}

Vector<string> Map::ReadJson(const string & path)
{
	std::ifstream file(path);
	if(!file.is_open())[[unlikely]]
	{
		throw std::invalid_argument("Could'nt open file: " + path );
	}
	json j = json::parse(file);

	Vector<string>asset_paths_		=	j["asset_paths"].template get<Vector<string>>();
	rows_							=	j["rows"].template get<u16>();
	cols_							=	j["cols"].template get<u16>();
	tilesize_						=	j["tilesize"].template get<u8>();
	layer_number_					=	j["layer_number"].template get<u8>();
	tiles_							=	j["tiles"].template get<Vector<Vector<Tile>>>();

	file.close();
	return asset_paths_;
}

void Map::WriteJson(const string & path)
{
	Vector<string> asset_paths;
	for(auto & n : tilesets_)
	{
		asset_paths.push_back(n.path);
	}
	const json dataJson{
			{"asset_paths", asset_paths},
			{"rows", rows_},
			{"cols", cols_},
			{"tilesize", tilesize_},
			{ "layer_number", layer_number_},
			{"tiles", tiles_}
	};

	std::ofstream file{path};
	if(!file)[[unlikely]]
	{
		throw std::invalid_argument("Error: Could not load file: " + path );
	}
	file <<  dataJson << std::endl;
	file.close();
}

void Map::CreateAssets(const Vector<string>& asset_paths, Renderer* render) {
	for(auto & vec : asset_paths)
	{
		string tmpstr = BasePath;
		tmpstr.append(vec);
		Point size;
		SharedPtr<Texture> texture_shptr = CreateSharedTexture(render, tmpstr.c_str(), &size);

		tilesets_.push_back({texture_shptr , size, vec});
	}
}


