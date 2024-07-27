//
// Created by bzl on 27.07.24.
//
#include "tilemap.h"

namespace BzlGame
{

void Map::ReadJson(const string & path, Renderer * render)
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
	layer_numb_						=	j["layer_numb"].template get<u8>();
	tiles_							=	j["tiles"].template get<Vector<Vector<Tile>>>();

	for(auto & vec : asset_paths_)
	{
		string tmpstr = BasePath;
		tmpstr.append(vec);
		SharedPtr<Texture> texture_shptr = CreateSharedTexture(render, tmpstr.c_str());
		Point size;
		SDL_QueryTexture(texture_shptr.get(), nullptr, nullptr, &size.x, &size.y );
		tilesets_.push_back({texture_shptr , size, vec});

	}
	file.close();
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
			{ "layer_numb", layer_numb_},
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




}