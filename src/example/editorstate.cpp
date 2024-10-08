#include <imgui.h>
#include <imgui_impl_sdl2.h>

#include <nlohmann/json.hpp>

#include "GUI.h"
#include "examplegame.h"
using namespace BzlGame;

void EditorState::Init()
{
	gui_texture_ = CreateSharedTexture(render, BasePath"asset/graphic/editorGUI.png");

	const int ntf_error = OpenFileDialog();
	if(ntf_error == 1)
	{
		print("NTF ERROR 1");
		Event next_event = { .type = SDL_QUIT };
		SDL_PushEvent( &next_event );
	}

	scaler_ = map_.tilesize() * zoom_;
	gui_ = std::make_unique<GUI>(&map_, this);

}

void EditorState::UnInit(){}

void EditorState::MoveCamera(const Direction dir)
{
    static constexpr u16 CameraSpeed = 4;
	Point winSize = game.GetWindowSize();
	switch (dir){
		case Direction::Up:
			if(camera_.y > CameraSpeed)				{		camera_.y -= CameraSpeed;		}
			break;
		case Direction::Down:
			if(camera_.y < winSize.y-CameraSpeed){		camera_.y += CameraSpeed;		}
			break;
		case Direction::Left:
			if(camera_.x > CameraSpeed)				{		camera_.x -= CameraSpeed;		}
			break;
		case Direction::Right:
			if(camera_.x < winSize.x-CameraSpeed){		camera_.x += CameraSpeed;		}
			break;
		default:
			break;
	}
}

/// @brief Open native file save dialog and save new map or overwrite an existing file
/// @return 0 for success, 1 for an error, 2 for user cancellation
int EditorState::SaveFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_SaveDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY ) {
		GetRelativePath(string(outPath), string(BasePath));
		map_.WriteJson(outPath);
		free(outPath);
		return 0;

	}
	if ( result == NFD_CANCEL ) {
		DebugOnly(		println("Canceled by user.");			)
		return 2;
	}
	DebugOnly(		printf("Error: %s\n", NFD_GetError() );	)
	return 1;
}

/// @brief Open native file dialog and select asset to load
/// @return 0 for success, 1 for an error, 2 for user cancellation
int EditorState::OpenAssetFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		Point assetsize;
		SharedPtr<Texture> asset_texture = CreateSharedTexture(render, outPath, &assetsize);
		if(asset_texture == nullptr)
		{
			return 1;
		}
		const string absolutpath = outPath;
		const string relativpath = RemovePathBeforeAsset(absolutpath);

		map_.AddTileset(asset_texture, assetsize, relativpath);
		free(outPath);

		return 0;
	}

	DebugOnly(		println("Canceled by user.");		)
	return 2;
}

/// @brief Open native file dialog and select map.json to load
/// @return 0 for success, 1 for an error, 2 for user cancellation
int EditorState::OpenFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );

	// Load Map from JSON
	if ( result == NFD_OKAY )
	{
		Vector<string> asset_paths = map_.ReadJson(outPath);
		map_.LoadAllAssets(asset_paths, render);

		free(outPath);
		return 0;
	} // create empty map
	if ( result == NFD_CANCEL )
	{
		DebugOnly(		println("Canceled by user. Load standard-map");		)
		map_ = Map();
		return 2;
	}
		return 1;
}

void EditorState::Events( const u32 frame, const u32 totalMSec, const float deltaT )
{
	SDL_PumpEvents();

	Event event;
	while( SDL_PollEvent( &event ) )
	{
#ifdef BZ_IMGUI_ACTIVE
		const ImGuiIO & io = ImGui::GetIO();
		ImGui_ImplSDL2_ProcessEvent(&event); // Pass events to ImGui
		if( io.WantCaptureKeyboard ){	continue;	}
		if( io.WantCaptureMouse ){	continue;	}

#endif

		if( game.HandleEvent( event ) )
			continue;

		switch( event.type )
		{
			case SDL_WINDOWEVENT:
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					// Aktualisiere die WindowSize-Variable
					Point windowSize;
					SDL_GetWindowSize(game.GetWindow(), &windowSize.x, &windowSize.y);
					panel.lower.w = windowSize.x;
					panel.upper.w = windowSize.x;
					panel.upper.h = windowSize.y / 2;
					panel.lower.h = windowSize.y - panel.upper.h;
					panel.lower.y = panel.upper.h;

				}
				break;
			}
			case SDL_KEYDOWN:
				{
					HandleKeyboard(event);
					break;
				}
			case SDL_MOUSEMOTION:
				{
					if(drag_cam_) {
						SDL_Point currentMousePos = {event.motion.x, event.motion.y};
						camera_.x += currentMousePos.x - camera_startpos.x;
						camera_.y += currentMousePos.y - camera_startpos.y;
						camera_startpos = currentMousePos;
					}

					if(leftclick_modctrl_)
					{
						mselect_endp_.x = event.motion.x;
						mselect_endp_.y = event.motion.y;
					}
					/// Update current mouse position
					grid_mousepos.x  = event.motion.x / scaler_;
					grid_mousepos.y  = event.motion.y / scaler_;
					break;
				}
			case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						// if mouse is inside LowerPanel, select tile on current mouse pos
						if( !isMouseOnMap()	&&	isAtlasVisible())
							{
								// check if ctrl is pressed for multiselect-start
								const Uint8* keystate = SDL_GetKeyboardState(nullptr);
								if ( keystate[SDL_SCANCODE_LCTRL])
								{
									leftclick_modctrl_ = true;
									mselect_startp_ = {event.button.x, event.button.y};
								}else{
									singleselect_point.x = grid_mousepos.x;
									singleselect_point.y = grid_mousepos.y - panel.lower.y / scaler_;
									multiselect_Items.clear();
								}
								break;
							}
						// if mouse is NOT in LowerPanel, place last Selected Tile on current Position.
						if( isMouseOnMap()	||	!isAtlasVisible()	)
						{
							if(multiselect_Items.empty())
							{
									const u16 selected_type = static_cast<u16>(pointToInt(singleselect_point, map_.getTilesetSize(tileset_id_).x / map_.tilesize()));
									const u16 dst_pos = pointToInt(grid_mousepos, map_.cols());
									map_.setTile(layer_id_, dst_pos, {selected_type, tileset_id_});
							}else
							{
								for(const auto & multiItem : multiselect_Items)
								{
									const u16 selected_type = static_cast<u16>(pointToInt(multiItem.tileset_pos, map_.getTilesetSize(tileset_id_).x / map_.tilesize()));
									const Point pos = {grid_mousepos.x + multiItem.offset.x, grid_mousepos.y + multiItem.offset.y};
									const int dst_pos = pointToInt(pos, map_.cols());
									map_.setTile(layer_id_, dst_pos, {selected_type, tileset_id_});
								}
							}
							break;
						}
					}else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						if(isMouseOnMap() && (SDL_GetModState() & KMOD_LALT))
						{
							drag_cam_ = true;
							camera_startpos.x = event.button.x;
							camera_startpos.y = event.button.y;
						}else
						{
							// if mouse is within the map, use right-click on a tile to replace it with EmptyTileVal.
							if(	isMouseOnMap()	)
							{
								const int dst_pos = pointToInt(grid_mousepos, map_.cols());
								map_.setTile(layer_id_, dst_pos, {EmptyTileVal, tileset_id_});
							}
						}
					}
					break;
				}
			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_LEFT)
				{
					if(event.button.y > panel.lower.y && leftclick_modctrl_)
					{
						// Set square to tileset coords
						const int startX = mselect_startp_.x / scaler_;
						const int startY = (mselect_startp_.y - panel.lower.y) / scaler_;
						const int endX   = mselect_endp_.x / scaler_;
						const int endY   = (mselect_endp_.y - panel.lower.y) / scaler_;

						multiselect_Items.clear();
						for (int x = std::min(startX, endX); x <= std::max(startX, endX); ++x)
						{
							for (int y = std::min(startY, endY); y <= std::max(startY, endY); ++y)
							{
								const Point tileset_pos = {x, y};
								Point offset = {x - startX, y - startY};
								multiselect_Items.push_back({tileset_pos, offset});
							}
						}
						leftclick_modctrl_ = false;
					}
					break;
				}
				if(event.button.button == SDL_BUTTON_RIGHT && drag_cam_)
				{
					drag_cam_ = false;
					break;
				}
			default:
				break;
		}
	}
}

string EditorState::GetRelativePath(const std::string &absolutePath, const std::string &projectPath) {
	namespace fs = std::filesystem;
	const fs::path absPath(absolutePath);
	fs::path projPath(projectPath);

	// check if projPath is absolut
	if (!projPath.is_absolute()) {
		projPath = absolute(projPath);
	}
	const fs::path relPath = relative(absPath, projPath);
	return relPath.string();
}

void EditorState::HandleKeyboard(const SDL_Event & event)
{
	const Keysym & what_key = event.key.keysym;
	if( what_key.scancode == SDL_SCANCODE_F5 && event.key.repeat == 0 ){
		SaveFileDialog();
	}
	else if(what_key.scancode == SDL_SCANCODE_UP)		{	MoveCamera(Direction::Up);		}
	else if(what_key.scancode == SDL_SCANCODE_DOWN)		{	MoveCamera(Direction::Down);	}
	else if(what_key.scancode == SDL_SCANCODE_LEFT)		{	MoveCamera(Direction::Left);	}
	else if(what_key.scancode == SDL_SCANCODE_RIGHT)	{	MoveCamera(Direction::Right);	}

	else if( what_key.scancode == SDL_SCANCODE_F2 && event.key.repeat == 0){
		if(isAtlasVisible())
		{
			atlasEnabled = false;
		}else
		{
			atlasEnabled = true;
		}
	}else if( what_key.scancode == SDL_SCANCODE_ESCAPE && event.key.repeat == 0){
		game.SetNextState( 0 );
	}
}

void EditorState::Update( const u32 frame, const u32 totalMSec, const float deltaT ){}

void EditorState::Render( const u32 frame, const u32 totalMSec, const float deltaT )
{
	RenderMap();
	if(isAtlasVisible()){
		RenderAtlas();
	}
	RenderMouse();
	gui_->RenderAll();
}

void EditorState::RenderMap() {
	const u16 tilesize = map_.tilesize();
	for( auto & lay : map_.getTileVec())
	{
		for(int tile = 0; tile < lay.size(); tile++)
		{
			assert(lay[tile].type <= EmptyTileVal);
			if(lay[tile].type != EmptyTileVal){
				// Calculate position from type in TileSet
				const Point tileset_pos = intToPoint(lay[tile].type, map_.getTilesetSize(lay[tile].asset_id).x / tilesize);
				const Rect srcRect
				{
					tileset_pos.x * tilesize,
					tileset_pos.y * tilesize,
					tilesize, tilesize
				};

				// Calculate destination on Map
				const Point dstCoords = intToPoint(tile,map_.cols());
				const Rect dstRect
				{
					dstCoords.x *scaler_ - camera_.x  ,
					dstCoords.y *scaler_ - camera_.y  ,
					scaler_, scaler_
				};

				// check if atlas panel is activated, if true, check for lower_panel rendering border
				if(isAtlasVisible())
				{
					if(dstRect.y+map_.tilesize() <= panel.lower.y)
					{
						SDL_RenderCopy(render, map_.getTileset(lay[tile].asset_id).texture.get(), &srcRect, &dstRect);
					}
				}else
				{
					SDL_RenderCopy(render, map_.getTileset(lay[tile].asset_id).texture.get(), &srcRect, &dstRect);
				}
			}
		}
	}
}

void EditorState::RenderAtlas(){
	if(!map_.getTilesets().empty()){
		SDL_SetRenderDrawColor(render, 100, 100, 100, 255);
		SDL_RenderFillRect(render, &panel.lower);

		const int tile_number = (map_.getTilesetSize(tileset_id_).x * map_.getTilesetSize(tileset_id_).y )/ map_.tilesize();
		for(int i = 0; i < tile_number; i++)
		{

			const Point relative_pos = intToPoint(i , map_.getTilesetSize(tileset_id_).x / map_.tilesize());
			const Rect srcRect =
			{
				relative_pos.x * map_.tilesize(),
				relative_pos.y * map_.tilesize(),
				map_.tilesize(), map_.tilesize()
			};
			const Rect dstRect =
			{
				relative_pos.x * scaler_,
			   panel.lower.y + relative_pos.y * scaler_,
			   scaler_, scaler_
		   };
			SDL_RenderCopy(render, map_.getTileset(tileset_id_).texture.get(), &srcRect, &dstRect);


		}
		RenderSelected();
	}
}

void EditorState::RenderSelected() const {

	// single selection
	if(multiselect_Items.empty())
	{
		const Rect mouse_srcRect = {0,0,scaler_,scaler_};
		const Rect dst_rect = {singleselect_point.x * scaler_, singleselect_point.y * scaler_ + panel.lower.y, scaler_, scaler_};
		SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &dst_rect );

	// multi selection
	}else
	{
		SDL_SetRenderDrawColor(render, 255,255,255,255); // white
		const Rect dst_rect = {
			(mselect_rect_.x / scaler_ ) * scaler_,
			(mselect_rect_.y   / scaler_) * scaler_,
			((mselect_rect_.w / scaler_) * scaler_ ) + map_.tilesize(),
			((mselect_rect_.h /scaler_ )* scaler_) + map_.tilesize()
		};

		SDL_RenderDrawRect(render, &dst_rect);
	}
}

void EditorState::RenderMouse() {

	if(leftclick_modctrl_ && isAtlasVisible())[[unlikely]]
	{
		// save the start coordinates of the mouse at the buttondown event and the end coordinates when the button is released.
		// depending on whether the rectangle is dragged upwards or downwards, start x/y or end x/y becomes the origin of the rectangle.
		mselect_rect_.x = std::min(mselect_startp_.x, mselect_endp_.x);
		mselect_rect_.y = std::min(mselect_startp_.y, mselect_endp_.y);
		mselect_rect_.w = std::abs(mselect_endp_.x - mselect_startp_.x);
		mselect_rect_.h = std::abs(mselect_endp_.y - mselect_startp_.y);

		SDL_SetRenderDrawColor(render, 255, 255, 255, 255); // Weiß
		SDL_RenderDrawRect(render, &mselect_rect_);
	}else {
		// Render actual MousePos;
		const Rect mouse_srcRect =	{0, 0, scaler_, scaler_	};
		const Rect mouse_dstRect =
		{
			grid_mousepos.x * scaler_,
			grid_mousepos.y * scaler_,
			scaler_, scaler_
		};
		SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &mouse_dstRect);
	}
}



string EditorState::RemovePathBeforeAsset(const string &filepath) {

	const string marker = "asset";
	size_t pos = filepath.find(marker);
	if (pos != string::npos) {
		return filepath.substr(pos);
	} else {
		// Falls "asset" nicht im Pfad gefunden wurde, den ursprünglichen Pfad zurückgeben
		return filepath;
	}
}

