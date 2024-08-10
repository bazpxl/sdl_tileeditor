#pragma once

#include <global.h>
#include <gamebase.h>
#include <tilemap.h>

namespace BzlGame {

	class ExampleGame;
	class EditorState;

	// save position in tileset and offset to the first MultiSelectItem / root item.
	// needed for stamp multi-tile insert
	struct MultiSelectItem {
		Point tileset_pos;
		Point offset;
	};

	enum class Direction
	{
		Up,
		Down,
		Left,
		Right
	};

	class ExampleGame final : public Game
	{
	public:
		ExampleGame();
		bool HandleEvent( const Event event ) override;
	};

	class EditorState : public GameState
	{
	protected:


		Vector<MultiSelectItem> multiselect_Items;
		SharedPtr<Texture> gui_texture_ = nullptr;

		Map map_;

		// rendering borders map/atlas
		Rect upper_panel_				=	{ 0,0,WindowSize.x,WindowSize.y / 2};
		Rect lower_panel_				=	{ 0,WindowSize.y / 2,WindowSize.x,WindowSize.y / 2};

		Rect  camera_					=	{0,0,WindowSize.x,WindowSize.y / 2};
		Point camera_startpos{};
		Point camera_endpos{};

		Rect  mselect_rect_{};
		Point mselect_startp_			=	{ 0, WindowSize.y / 2 };
		Point mselect_endp_				=	{ 0, WindowSize.y / 2 };

		Point singleselect_point		=   { 0, 0};
		Point fixmousepos_				=	{ 0, 0};

		u16 scaler_{};
		u8 zoom_						=	1;

		u8 layer_id_					=	LayerNumber - 1;
		u8 tileset_id_					=	0;

		bool atlas_open_				=	true;
		bool leftclick_modctrl_			=	false;
		bool rightclick_				=	false;

	public:
		// ctor
		using GameState::GameState;

		[[nodiscard]] bool isAtlasVisible()  const	{   return atlas_open_;		}
		[[nodiscard]] bool isMouseOnMap()	 const	{	return ((fixmousepos_.y * scaler_) < lower_panel_.y);	}

		static string GetRelativePath(const string& absolutePath, const string& projectPath);
		static string RemovePathBeforeAsset(const string& filepath);

		void HandleKeyboard(const SDL_Event & event);
		void MoveCamera(const Direction dir);

		int OpenFileDialog();

		int SaveFileDialog();

		int OpenAssetFileDialog();

		void RenderMap();
		void RenderAtlas();
		void RenderSelected() const;
		void RenderMouse();
		void RenderGui_Menubar();
		void RenderGui();

		void Init()																override;
		void UnInit()															override;
		void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
		void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
		void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;

	};
}
