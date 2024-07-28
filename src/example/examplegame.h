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
		const int HalfWindowHeight = WindowSize.y / 2;

		Vector<MultiSelectItem> multiselect_Items;
		SharedPtr<Texture> gui_texture_ = nullptr;

		Map map_;

		Rect multiselection_{};
		Rect camera_map_				=	{0,0,WindowSize.x,HalfWindowHeight};

		// rendering borders map/atlas
		Rect upper_panel_				=	{ 0,0,WindowSize.x,HalfWindowHeight};
		Rect lower_panel_				=	{ 0,HalfWindowHeight,WindowSize.x,HalfWindowHeight};

		Point singleselect_point		=   { 0, 0};
		Point mselect_startp_			=	{ 0, HalfWindowHeight };
		Point mselect_endp_				=	{ 0, HalfWindowHeight };
		Point fixmousepos_				=	{ 0, 0};

		u16 scaler_{};
		u8 zoom_						=	1;

		u8 layer_id_					=	LayerNumb - 1;
		u8 tileset_id_					=	0;

		bool atlas_open_				=	true;
		bool mouse_modctrl				=	false;

		[[nodiscard]] bool isAtlasVisible()  const		{   return atlas_open_;		}



		std::string GetRelativePath(const std::string& absolutePath, const std::string& projectPath);

		void InputKeyboard(const SDL_Event & event);
		void MoveCamera(const Direction dir);

		void OpenFileDialog();
		void SaveFileDialog();
		void OpenAssetFileDialog();

		void RenderMap();
		void RenderAtlas();
		void RenderMouse();
		void RenderGUI();

	public:
		// ctor
		using GameState::GameState;

		static std::string RemovePathBeforeAsset(const std::string& filepath);

		void Init()																override;
		void UnInit()															override;
		void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
		void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
		void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;

	};
}
