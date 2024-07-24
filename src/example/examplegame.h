#pragma once

#include <global.h>
#include <gamebase.h>
#include <tilemap.h>

class ExampleGame;
class EditorState;
using namespace BzlGame;



class ExampleGame final : public Game
{
public:
	ExampleGame();

	bool HandleEvent( const Event event ) override;
};



class EditorState : public GameState
{
protected:
	// width/height from tilesets
	Array<Point, 10> tset_size_array_{{0,0}};

	SharedPtr<Texture> gui_texture_ = nullptr;
	Vector<Point> multiselect_points_;

	MapHeader map_header_{};
	MapData map_data_{};

	u8 zoom_					=	2;
	u16 scaled_size_			=	zoom_ * TileSize;
	Rect camera_map_			=	{0,0,WindowSize.x,WindowSize.x / 2};

	Rect upper_panel_		    =	{0,0,WindowSize.x,WindowSize.y / 2};
	Rect lower_panel_			=	{0,WindowSize.y / 2,WindowSize.x,WindowSize.y / 2	};

	Point multiselect_startpos_		=	{ 0, WindowSize.y / 2};
	Point multiselect_endpos_		=	{	0,	WindowSize.y / 2 };

	Point mousepos_				=	{0,	0};

	u8 layer_id_				=	LayerNumb - 1;
	u8 tileset_id_				=	0;

	bool atlas_open_			=	true;
	bool mouseButtonPressed_		=	false;


	enum class Direction
	{
		Up,
		Down,
		Left,
		Right
	};

public:
	// ctor
	using GameState::GameState;

	[[nodiscard]] bool isAtlasVisible() const { return atlas_open_;	}

	void InputKeyboard(const SDL_Event & event);

	void MoveCamera(const Direction dir);

	void CalculateSelectedTiles();

	void OpenFileDialog();
	void SaveFileDialog();
	void ReadJSON(const string& path) ;
	void WriteJSON(const string& path);

	void RenderMap() const;
	void RenderAtlas() const;
	void RenderGUI();

	void Init()																override;
	void UnInit()															override;
	void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;

};

