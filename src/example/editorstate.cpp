#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <nlohmann/json.hpp>

#include "examplegame.h"

void EditorState::Init()
{
	gui_texture_ = CreateSharedTexture(render, BasePath"asset/graphic/editorGUI.png");

	OpenFileDialog();

	for(int i = 0; i < map_data_.tileSets.size(); i++)
	{
		SDL_QueryTexture(map_data_.tileSets[i].get(), nullptr, nullptr,&tset_size_array_[i].x, &tset_size_array_[i].y );
		tset_size_array_[i].x = tset_size_array_[i].x;
		tset_size_array_[i].y = tset_size_array_[i].y;
	}

}

void EditorState::UnInit()
{

}

void EditorState::MoveCamera(Direction dir)
{
	switch (dir)
	{
		case Direction::Up:
			if(camera_map_.y > CameraSpeed)
			{
				camera_map_.y -= CameraSpeed;
			}
			break;
		case Direction::Down:
			if(camera_map_.y < WindowSize.y-CameraSpeed)
			{
				camera_map_.y += CameraSpeed;
			}
			break;
		case Direction::Left:
			if(camera_map_.x > CameraSpeed)
			{
				camera_map_.x -= CameraSpeed;
			}
			break;
		case Direction::Right:
			if(camera_map_.x < WindowSize.x-CameraSpeed)
			{
				camera_map_.x += CameraSpeed;
			}
			break;
		default:
			break;
	}
}

void EditorState::ReadJSON(const string & path)
{

	std::ifstream file(path);
	if(!file.is_open())[[unlikely]]
	{
		throw std::invalid_argument("Could'nt open file: " + path );
	}
	json j = json::parse(file);

	map_header_.asset_paths		=	j["asset_paths"].template get<Vector<string>>();
	map_header_.rows			=	j["rows"].template get<u16>();
	map_header_.cols			=	j["cols"].template get<u16>();
	map_header_.tilesize		=	j["tilesize"].template get<u8>();
	map_header_.layer_numb		=	j["layer_numb"].template get<u8>();

	map_data_.tiles				=	j["tiles"].template get<Vector<Vector<Tile>>>();

	for(auto & vec : map_header_.asset_paths)
	{
		string tmpstr = BasePath;
		tmpstr.append(vec);
		map_data_.tileSets.push_back(CreateSharedTexture(render, tmpstr.c_str()));

	}
	file.close();
}

void EditorState::WriteJSON(const string & path)
{
	const json dataJson{
		{"asset_paths", map_header_.asset_paths},
		{"rows", map_header_.rows},
		{"cols", map_header_.cols},
		{"tilesize", map_header_.tilesize},
		{ "layer_numb", map_header_.layer_numb},
		{"tiles", map_data_.tiles}
	};

	std::ofstream file{path};
	if(!file)[[unlikely]]
	{
		throw std::invalid_argument("Error: Could not load file: " + path );
	}
	file <<  dataJson << std::endl;
	file.close();

}

void EditorState::SaveFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_SaveDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		WriteJSON(outPath);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		println("Canceled by user.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError() );
	}
}

void EditorState::OpenFileDialog()
{
	nfdchar_t *outPath = nullptr;
	const nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		ReadJSON(outPath);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		println("Canceled by user. Load standard-map");
		ReadJSON( BasePath"asset/map.json" );
	}
	else{
		println("Error: {}", NFD_GetError() );
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
					InputKeyboard(event);
					break;
				}

			case SDL_MOUSEMOTION:
				{
					if(mouseButtonPressed_)
					{
						multiselect_endpos_.x = event.motion.x;
						multiselect_endpos_.y = event.motion.y;
					}
					/// Update current mouse position
					mousepos_.x  = event.motion.x / scaled_size_;
					mousepos_.y  = event.motion.y / scaled_size_;
					break;
				}
			case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{

						// if mouse is inside LowerPanel, select tile on current mouse pos
						if((mousepos_.y * scaled_size_  >= lower_panel_.y)	&&	isAtlasVisible())
							{
								// check if ctrl is pressed for multiselect-start
								const Uint8* keystate = SDL_GetKeyboardState(nullptr);
								if (keystate[SDL_SCANCODE_LCTRL])
								{
									mouseButtonPressed_ = true;
									multiselect_startpos_ = {event.button.x, event.button.y};
								}else
								{
									select_point.x = mousepos_.x;
									select_point.y = mousepos_.y - lower_panel_.y / scaled_size_;
									multiselect_points_.clear();
								}

							}

						// if mouse is NOT in LowerPanel, place last Selected Tile on current Position.
						if((mousepos_.y * scaled_size_ < lower_panel_.y)	||	!isAtlasVisible()	)
							{
								if(multiselect_points_.empty())
								{
									const int selected_type = static_cast<u16>(pointToInt(select_point, tset_size_array_[tileset_id_].x  / TileSize));
									const int dst_pos = pointToInt(mousepos_, map_header_.cols);
									map_data_.tiles[layer_id_][dst_pos].type = selected_type;
									map_data_.tiles[layer_id_][dst_pos].asset_id = tileset_id_;
								}else {
									for(auto & multiItem : multiselect_points_)
									{
										const int selected_type = static_cast<u16>(pointToInt(multiItem.tileset_pos, tset_size_array_[tileset_id_].x  / TileSize));
										Point pos = {mousepos_.x + multiItem.offset.x, mousepos_.y + multiItem.offset.y};
										const int dst_pos = pointToInt(pos, map_header_.cols);
										map_data_.tiles[layer_id_][dst_pos].type = selected_type;
										map_data_.tiles[layer_id_][dst_pos].asset_id = tileset_id_;
									}
								}
							}
					}else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						// if mouse is in valid Panel, click to remove tile type and replace it with EmptyTileVal.
						if((mousepos_.y * scaled_size_ < lower_panel_.y) || !isAtlasVisible()	)
						{
							const int dst_pos = pointToInt(mousepos_, map_header_.cols);
							map_data_.tiles[layer_id_][dst_pos].type = EmptyTileVal;
						}
					}
					break;
				}
			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_LEFT)
				{

					if(event.button.y > lower_panel_.y && mouseButtonPressed_){
						//println("Multiselection rect: x {} y {} w {} h {}", multi_selection_rect_.x, multi_selection_rect_.y, multi_selection_rect_.w, multi_selection_rect_.h);
						const int startX = multi_selection_rect_.x;
						const int startY = multi_selection_rect_.y;
						const int endX = multi_selection_rect_.w;
						const int endY = multi_selection_rect_.h;

						multiselect_points_.clear();
						for(int x = startX; x <= endX; x++) {
							for(int y = startY; y <= endY; y++) {
								if(multiselect_points_.empty()) {
									multiselect_points_.push_back({{x,y}, {0,0}});
								}
								multiselect_points_.push_back({{x,y},{x-multiselect_points_[0].tileset_pos.x, y-multiselect_points_[0].tileset_pos.y}});
								//println("x {} y {}, offsetx {} offsety {}",x,y, x-multiselect_points_[0].tileset_pos.x, y-multiselect_points_[0].tileset_pos.y);
							}
						}

						mouseButtonPressed_ = false;
					}



				}

			default:
				break;
		}
	}
}

void EditorState::InputKeyboard(const SDL_Event & event)
{
	const Keysym & what_key = event.key.keysym;
	if( what_key.scancode == SDL_SCANCODE_F5 && event.key.repeat == 0 ){
		SaveFileDialog();
	}

	else if(what_key.scancode == SDL_SCANCODE_UP){
		MoveCamera(Direction::Up);
	}
	else if(what_key.scancode == SDL_SCANCODE_DOWN){
		MoveCamera(Direction::Down);
	}
	else if(what_key.scancode == SDL_SCANCODE_LEFT){
		MoveCamera(Direction::Left);
	}
	else if(what_key.scancode == SDL_SCANCODE_RIGHT){
		MoveCamera(Direction::Right);
	}

	else if( what_key.scancode == SDL_SCANCODE_F2 && event.key.repeat == 0){
		if(isAtlasVisible()){
			atlas_open_ = false;
		}else{
			atlas_open_ = true;
		}
	}

	else if( what_key.scancode == SDL_SCANCODE_ESCAPE && event.key.repeat == 0){
		game.SetNextState( 0 );
	}
}

void EditorState::Update( const u32 frame, const u32 totalMSec, const float deltaT )
{

}

void EditorState::Render( const u32 frame, const u32 totalMSec, const float deltaT )
{
	RenderMap();

	if(atlas_open_){
		RenderAtlas();
	}
	RenderMouse();
	RenderGUI();

}

void EditorState::RenderMap() const {
	for(const auto & lay : map_data_.tiles)
	{
		for(int tile = 0; tile < lay.size(); tile++)
		{
			assert(lay[tile].type <= EmptyTileVal);
			if(lay[tile].type != EmptyTileVal){
				// Calculate position from type in TileSet
				const Point tileset_pos = intToPoint(lay[tile].type, tset_size_array_[lay[tile].asset_id].x / TileSize);
				const Rect srcRect
				{
					tileset_pos.x * map_header_.tilesize,
					tileset_pos.y * map_header_.tilesize,
					map_header_.tilesize, map_header_.tilesize
				};

				// Calculate destination on Map
				const Point dstCoords = intToPoint(tile,map_header_.rows);
				const Rect dstRect
				{
					dstCoords.x *scaled_size_ - camera_map_.x  ,
					dstCoords.y *scaled_size_ - camera_map_.y  ,
					scaled_size_, scaled_size_
				};

				if(atlas_open_)
				{
					if(dstRect.y+TileSize < lower_panel_.y)
					{
						SDL_RenderCopy(render, map_data_.tileSets[lay[tile].asset_id].get(), &srcRect, &dstRect);
					}
				}else
				{
					SDL_RenderCopy(render, map_data_.tileSets[lay[tile].asset_id].get(), &srcRect, &dstRect);
				}
			}
		}
	}
}

void EditorState::RenderAtlas() const
{
	assert(WindowSize.x > tset_size_array_[tileset_id_].x);
	assert(WindowSize.y > tset_size_array_[tileset_id_].y);

		const int tileNumb = tset_size_array_[tileset_id_].x * tset_size_array_[tileset_id_].y / TileSize;
		for(int i = 0; i < tileNumb; i++)
		{
			const Point relative_pos = intToPoint(i , tset_size_array_[tileset_id_].x / map_header_.tilesize);
			const Rect srcRect =
			{
				relative_pos.x * map_header_.tilesize,
				relative_pos.y * map_header_.tilesize,
				map_header_.tilesize, map_header_.tilesize
			};
			const Rect dstRect =
			{
				relative_pos.x * scaled_size_,
			   lower_panel_.y + relative_pos.y * scaled_size_,
			   scaled_size_, scaled_size_
		   };

			SDL_RenderCopy(render, map_data_.tileSets[tileset_id_].get(), &srcRect, &dstRect);


			if(multiselect_points_.empty())
			{
				const Rect mouse_srcRect = {0,0,scaled_size_,scaled_size_};
				const Rect dst_rect = {select_point.x * scaled_size_, select_point.y * scaled_size_ + lower_panel_.y, scaled_size_, scaled_size_};
				SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &dst_rect );
			}else
			{
				for(const auto & multiItem: multiselect_points_){
					const Rect mouse_srcRect = {0,0,scaled_size_,scaled_size_};
					const Rect dst_rect = {multiItem.tileset_pos.x * scaled_size_, multiItem.tileset_pos.y * scaled_size_ + lower_panel_.y, scaled_size_, scaled_size_};
					SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &dst_rect );
				}
			}


		}
}

void EditorState::RenderMouse() {
	// Render actual MousePos;
	const Rect mouse_srcRect =	{0, 0, scaled_size_, scaled_size_	};
	const Rect mouse_dstRect =
	{
		mousepos_.x * scaled_size_,
		mousepos_.y * scaled_size_,
		scaled_size_, scaled_size_
	};
	SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &mouse_dstRect);

	if(mouseButtonPressed_){
		{
			// set multiselection square to render coords
			multi_selection_rect_.x = std::min(multiselect_startpos_.x, multiselect_endpos_.x);
			multi_selection_rect_.y = std::min(multiselect_startpos_.y, multiselect_endpos_.y);
			multi_selection_rect_.w = std::abs(multiselect_endpos_.x - multiselect_startpos_.x);
			multi_selection_rect_.h = std::abs(multiselect_endpos_.y - multiselect_startpos_.y);

			SDL_SetRenderDrawColor(render, 255, 255, 255, 255); // Weiß
			SDL_RenderDrawRect(render, &multi_selection_rect_);

			// set square to tileset coords
			multi_selection_rect_.x = multi_selection_rect_.x / scaled_size_;
			multi_selection_rect_.y = (multi_selection_rect_.y - lower_panel_.y) / scaled_size_;
			multi_selection_rect_.w = multi_selection_rect_.w / scaled_size_;
			multi_selection_rect_.h = multi_selection_rect_.h / scaled_size_;
		}
	}
}

void EditorState::RenderGUI()
{
#ifdef IMGUI

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();


	if(game.imgui_window_active)
	{
		ImGui::Begin("ImGUI window", &game.imgui_window_active);
		ImGui::Checkbox("Show Atlas", &atlas_open_);

		constexpr int MaxSize = 650;
		constexpr int MinSize = 0;
		// Control panel sizes
		ImGui::SliderInt("Atlas height", &lower_panel_.h, MinSize, MaxSize);
		lower_panel_.y = ((MaxSize - lower_panel_.h) / scaled_size_ )* scaled_size_;
		upper_panel_.h = WindowSize.y - lower_panel_.h;

		// Control current target Layer
		int slider_layer = layer_id_;
		ImGui::SliderInt("active layer:", &slider_layer, 0, LayerNumb-1);
		layer_id_ = static_cast<u8>(slider_layer);

		int slider_zoom = zoom_;
		ImGui::SliderInt("Render scale", &slider_zoom, 1, 3);
		zoom_ = static_cast<u8>(slider_zoom);
		scaled_size_ = zoom_ * map_header_.tilesize;

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
