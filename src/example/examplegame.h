#pragma once

#include <global.h>
#include <gamebase.h>
#include <tilemap.h>



namespace BzlGame {

	class ExampleGame;
	class EditorState;
	class GUI;

	// save position in tileset and offset to the first MultiSelectItem / root item.
	// needed for stamp multi-tile insert
	struct MultiSelectItem {
		Point tileset_pos;
		Point offset;
	};

	struct Panels {
		Rect upper;
		Rect lower;
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
		std::unique_ptr<GUI>gui_;

		// rendering borders map/atlas
		Panels panel = {{ 0,0,WindowSize.x,WindowSize.y / 2},
						{ 0,WindowSize.y / 2,WindowSize.x,WindowSize.y / 2}};

		Rect  camera_					=	{0,0,WindowSize.x,WindowSize.y / 2};
		Point camera_startpos{};
		Point camera_endpos{};

		Rect  mselect_rect_{};
		Point mselect_startp_			=	{ 0, WindowSize.y / 2 };
		Point mselect_endp_				=	{ 0, WindowSize.y / 2 };

		Point singleselect_point		=   { 0, 0};
		Point grid_mousepos				=	{ 0, 0};

		u16 scaler_{};
		u8 zoom_						=	1;

		u8 layer_id_					=	LayerNumber - 1;
		u8 tileset_id_					=	0;

		bool drag_cam_					=	false;
		bool atlasEnabled				=	true;
		bool leftclick_modctrl_			=	false;

	public:

		// ctor
		using GameState::GameState;

		[[nodiscard]] bool isAtlasVisible()  const	{   return atlasEnabled;		}
		[[nodiscard]] bool isMouseOnMap()	 const	{	return (grid_mousepos.y * scaler_) < panel.lower.y;	}

		static string GetRelativePath(const string& absolutePath, const string& projectPath);
		static string RemovePathBeforeAsset(const string& filepath);

		void HandleKeyboard(const SDL_Event & event);
		void MoveCamera(const Direction dir);

		void SetPanelHeight(const int slider) {
			panel.upper.h = slider;
			panel.lower.y = (panel.upper.h / scaler_) * scaler_;
			panel.lower.h = game.GetWindowSize().y - panel.lower.y;
		}
		void SetUpperPanel(const Rect &upper_panel)		{	panel.upper = upper_panel;	}
		void SetLowerPanel(const Rect &lower_panel)		{	panel.lower = lower_panel;	}
		void SetScaler(const u16 scaler)				{	scaler_ = scaler;			}
		void SetZoom(const u8 zoom)						{	zoom_ = zoom;				}
		void SetTileSetID(const u8 tileset_id)			{	tileset_id_ = tileset_id;	}
		void SetLayerID(const u8 layer_id)				{	layer_id_ = layer_id;		}

		[[nodiscard]] Renderer * GetRenderer()  const {	return game.GetRenderer();		}
		[[nodiscard]] Panels GetPanels()		const {	return panel;					}
		[[nodiscard]] Point GetWindowSize()		const { return game.GetWindowSize();	}
		[[nodiscard]] Point GetGridMousepos()	const {	return grid_mousepos;			}
		[[nodiscard]] u16 GetScaler()			const {	return scaler_;					}
		[[nodiscard]] u8 GetZoom()				const {	return zoom_;					}
		[[nodiscard]] u8 GetLayerID()			const {	return layer_id_;				}
		[[nodiscard]] u8 GetTilesetID()			const {	return tileset_id_;				}
		[[nodiscard]] bool & refAtlasEnabled()		  {	return atlasEnabled;			}

		int OpenFileDialog();
		int SaveFileDialog();
		int OpenAssetFileDialog();

		void RenderMap();
		void RenderAtlas();
		void RenderSelected() const;
		void RenderMouse();

		void Init()																override;
		void UnInit()															override;
		void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
		void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
		void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;

	};
}
