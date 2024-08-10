
#include "tilemap.h"
#include <filesystem>
using namespace BzlGame;

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
	layer_number_						=	j["layer_numb"].template get<u8>();
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
			{ "layer_numb", layer_number_},
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


