#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <nlohmann/json.hpp>

#include "examplegame.h"
using namespace BzlGame;


void EditorState::Init()
{
	gui_texture_ = CreateSharedTexture(render, BasePath"asset/graphic/editorGUI.png");
	OpenFileDialog();
	scaler_ = map_.tilesize() * zoom_;
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


void EditorState::SaveFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_SaveDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY ) {
		GetRelativePath(string(outPath), string(BasePath));
		map_.WriteJson(outPath);
		free(outPath);
	} else if ( result == NFD_CANCEL ) {
		DebugOnly(
			println("Canceled by user.");)
	} else{
		DebugOnly(
		printf("Error: %s\n", NFD_GetError() );	)
	}
}
void EditorState::OpenAssetFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		SharedPtr<Texture> asset_texture = CreateSharedTexture(render, outPath);
		assert(asset_texture != nullptr);
		Point size;

		const string absolutpath = outPath;
		const string relativpath = RemovePathBeforeAsset(absolutpath);

		SDL_QueryTexture(asset_texture.get(), nullptr, nullptr, &size.x, &size.y);
		map_.AddTileset(asset_texture, size, relativpath);
		free(outPath);
	}
	else if ( result == NFD_CANCEL ){
		DebugOnly(
		println("Canceled by user. Load standard-map");	)
		map_ = Map();
	}else{
		DebugOnly(
		println("Error: {}", NFD_GetError() );	)
	}
}
void EditorState::OpenFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );
	if ( result == NFD_OKAY )
	{
		Vector<string> asset_paths = map_.ReadJson(outPath);
		map_.CreateAssets(asset_paths, render);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		DebugOnly(
		println("Canceled by user. Load standard-map");	)
		map_ = Map();
	}
	else{
		DebugOnly(
		println("Error: {}", NFD_GetError() );	)
	}
}

void EditorState::Events( const u32 frame, const u32 totalMSec, const float deltaT )
{
	SDL_PumpEvents();

	Event event;
	while( SDL_PollEvent( &event ) )
	{
#ifdef IMGUI
		const ImGuiIO & io = ImGui::GetIO();
		ImGui_ImplSDL2_ProcessEvent(&event); // Pass events to ImGui
		if( io.WantCaptureKeyboard ){	continue;	}
		if( io.WantCaptureMouse ){	continue;	}

#endif

		if( game.HandleEvent( event ) )
			continue;

		switch( event.type )
		{
			case SDL_KEYDOWN:
				{
					HandleKeyboard(event);
					break;
				}
			case SDL_MOUSEMOTION:
				{
					if(leftclick_modctrl_)
					{
						mselect_endp_.x = event.motion.x;
						mselect_endp_.y = event.motion.y;
					}
					/// Update current mouse position
					fixmousepos_.x  = event.motion.x / scaler_;
					fixmousepos_.y  = event.motion.y / scaler_;
					break;
				}
			case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						// if mouse is inside LowerPanel, select tile on current mouse pos
						if((fixmousepos_.y * scaler_  >= lower_panel_.y)	&&	isAtlasVisible())
							{
								// check if ctrl is pressed for multiselect-start
								const Uint8* keystate = SDL_GetKeyboardState(nullptr);
								if ( keystate[SDL_SCANCODE_LCTRL])
								{
									leftclick_modctrl_ = true;
									mselect_startp_ = {event.button.x, event.button.y};
								}else{
									singleselect_point.x = fixmousepos_.x;
									singleselect_point.y = fixmousepos_.y - lower_panel_.y / scaler_;
									multiselect_Items.clear();
								}
							}
						// if mouse is NOT in LowerPanel, place last Selected Tile on current Position.
						if((fixmousepos_.y * scaler_ < lower_panel_.y)	||	!isAtlasVisible()	)
							{
								if(multiselect_Items.empty())
								{
									const u16 selected_type = static_cast<u16>(pointToInt(singleselect_point, map_.getTilesetSize(tileset_id_).x / map_.tilesize()));
									const int dst_pos = pointToInt(fixmousepos_, map_.cols());
									map_.setTile(layer_id_, dst_pos, {selected_type, tileset_id_});
								}else
								{
									for(const auto & multiItem : multiselect_Items)
									{
										const u16 selected_type = static_cast<u16>(pointToInt(multiItem.tileset_pos, map_.getTilesetSize(tileset_id_).x / map_.tilesize()));
										const Point pos = {fixmousepos_.x + multiItem.offset.x, fixmousepos_.y + multiItem.offset.y};
										const int dst_pos = pointToInt(pos, map_.cols());
										map_.setTile(layer_id_, dst_pos, {selected_type, tileset_id_});
									}
								}
							}
					}else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						// if mouse is within the map, use right click on a tile to replace it with EmptyTileVal.
						if(	isMouseOnMap() || !isAtlasVisible()	)
						{
							const int dst_pos = pointToInt(fixmousepos_, map_.cols());
							map_.setTile(layer_id_, dst_pos, {EmptyTileVal, tileset_id_});
							// if
						}else if(!isMouseOnMap() || isAtlasVisible())
						{
							rightclick_ = true;
							camera_startpos.x = event.button.x;
							camera_startpos.y = event.button.y;
						}
					}
					break;
				}
			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_LEFT)
				{
					if(event.button.y > lower_panel_.y && leftclick_modctrl_){
						// Set square to tileset coords
						const int startX = mselect_startp_.x / scaler_;
						const int startY = (mselect_startp_.y - lower_panel_.y) / scaler_;
						const int endX   = mselect_endp_.x / scaler_;
						const int endY   = (mselect_endp_.y - lower_panel_.y) / scaler_;

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
						//println("");
						leftclick_modctrl_ = false;
					}
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
		projPath = fs::absolute(projPath);
	}
	const fs::path relPath = fs::relative(absPath, projPath);
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
			atlas_open_ = false;
		}else
		{
			atlas_open_ = true;
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
	RenderGUI();
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
					if(dstRect.y+map_.tilesize() < lower_panel_.y)
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

void EditorState::RenderAtlas()
{

	if(!map_.getTilesets().empty()){
		const int tileNumb = (map_.getTilesetSize(tileset_id_).x * map_.getTilesetSize(tileset_id_).y )/ map_.tilesize();
		for(int i = 0; i < tileNumb; i++)
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
			   lower_panel_.y + relative_pos.y * scaler_,
			   scaler_, scaler_
		   };
			SDL_RenderCopy(render, map_.getTileset(tileset_id_).texture.get(), &srcRect, &dstRect);


		}
	}
}

void EditorState::RenderSelected() const {

	// single selection
	if(multiselect_Items.empty())
	{
		const Rect mouse_srcRect = {0,0,scaler_,scaler_};
		const Rect dst_rect = {singleselect_point.x * scaler_, singleselect_point.y * scaler_ + lower_panel_.y, scaler_, scaler_};
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

	if(leftclick_modctrl_ && isAtlasVisible())
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
			fixmousepos_.x * scaler_,
			fixmousepos_.y * scaler_,
			scaler_, scaler_
		};
		SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &mouse_dstRect);
	}
}

void EditorState::RenderGUI()
{
#ifdef IMGUI
	constexpr int MaxSize = WindowSize.y;
	constexpr int MinSize = 0;

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
	//ImGui::ShowUserGuide();
	ImGui::Begin("ImGUI window", &game.imgui_window_active);
	if(game.imgui_window_active)
	{
		if (ImGui::BeginListBox("Tilesets##setlistbox")) {
			for (int i = 0; i < map_.getTilesets().size(); ++i)
			{
				const bool is_selected = (i == tileset_id_); // Beispiel: markiere das erste Element
				if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
				{
					tileset_id_ = i;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndListBox();
		ImGui::NewLine();
		if(ImGui::Button("add")) {
			OpenAssetFileDialog();
		};
		ImGui::SameLine();
		if(ImGui::Button("delete")) {
			map_.RemoveTileset(layer_id_);
		}

		ImGui::Separator();

		// atlas panel slider
		ImGui::SliderInt("Atlas height", &lower_panel_.h, MinSize, MaxSize);
		lower_panel_.y = ((MaxSize - lower_panel_.h) / scaler_ )* scaler_;
		upper_panel_.h = WindowSize.y - lower_panel_.h;

		ImGui::Checkbox("show/hide##atlas", &atlas_open_);
		ImGui::NewLine();

		// scaling slider
		int slider_zoom = zoom_;
		ImGui::SliderInt("Render scale", &slider_zoom, 1, 3);
		zoom_ = static_cast<u8>(slider_zoom);
		scaler_ = zoom_ * map_.tilesize();

		ImGui::NewLine();

		if (ImGui::BeginCombo("Layer",std::to_string(layer_id_).c_str())) {
			for (int i = 0; i < map_.getTileVec().size(); ++i)
			{
				const bool is_selected = (i == layer_id_);
				if (ImGui::Selectable(std::to_string(i).c_str(), is_selected))
				{
					layer_id_ = i;
				}
			}
			ImGui::EndCombo();
		}

		//ImGui::NewLine();
		//ImGui::Button("add");

		//ImGui::SameLine();
		//ImGui::Button("delete" );

		ImGui::NewLine();
		if (ImGui::Button("save map")) {
			SaveFileDialog();
		}

		ImGui::End();
		ImGui::Render();
		SDL_SetRenderDrawColor(render, static_cast <u8>(0.45f * 255), static_cast <u8>(0.55f * 255), static_cast <u8>(0.60f * 255), static_cast <u8>(1.00f * 255));

		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
	}
#endif
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

