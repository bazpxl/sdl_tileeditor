#pragma once

#include <global.h>

#include <gamebase.h>
#include "recthelper.h"
#include <tilemap.h>


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

	SharedPtr<Texture> gui_texture = nullptr;

	MapHeader m_header;
	MapData m_data;

	Font    * pfont			= nullptr;

	Array<Point, 10> tset_size_array{{0,0}};		// width/height of the different assets/tilesets

	u8 zoom					= 2;
	const u16 scaledTileSize = zoom * TileSize;
	Rect camera_map		= {0,0,WindowSize.x,7*scaledTileSize};

	Rect LowerPanel   =  {	0,7*scaledTileSize, WindowSize.x, 7*scaledTileSize };

	Point mouseposition		= {0,0};
	Point selectedtile		= {0, LowerPanel.y};

	u8 tileset_id			= 0;
	bool atlas_open			= true;



	[[nodiscard]] bool isAtlasVisible() const { return atlas_open;	};

public:
	// ctor
	using GameState::GameState;

	void Init() override;
	void UnInit() override;



	void SaveFileDialog();
	void OpenFileDialog();

	void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void RenderAtlas() const;
	void RenderGUI();
};

