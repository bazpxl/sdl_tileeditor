#pragma once

#include <global.h>
#include <gamebase.h>
#include <tilemap.h>
#include "nlohmann/json.hpp"

class ExampleGame;
class IntroState;
using namespace BzlGame;

class ExampleGame final : public Game
{
public:
	ExampleGame();

	bool HandleEvent( const Event event ) override;
};


class IntroState : public GameState
{
protected:
	// width/height of the different assets/tilesets
	Array<Point, 10> tset_size_array_{{0,0}};

	SharedPtr<Texture> gui_texture_ = nullptr;

	MapHeader map_header_{};
	MapData map_data_{};

	u8 zoom_					= 2;
	const u16 scaled_size_		= zoom_ * TileSize;
	Rect camera_map_			= {0,0,WindowSize.x,WindowSize.x / 2};

	Rect upper_panel_		    =  {	0, 0, WindowSize.x, WindowSize.y / 2 };
	Rect lower_panel_			=  {	0,WindowSize.y / 2, WindowSize.x, WindowSize.y / 2 };

	Point mousepos_				= {0,0};
	Point selected_pos_			= {0, lower_panel_.y};

	u8 layer_id_				= LayerNumb - 1;
	u8 tileset_id_				= 0;

	bool atlas_open_			= true;

public:
	// ctor
	using GameState::GameState;

	[[nodiscard]] bool isAtlasVisible() const { return atlas_open_;	};

	void Init() override;
	void UnInit() override;

	void OpenFileDialog();
	void readJson(const string& path);

	void SaveFileDialog();
	void writeJson(const string& path);

	void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void RenderAtlas() const;
	void RenderGUI();
};

