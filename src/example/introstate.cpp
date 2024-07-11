#include "examplegame.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>



void IntroState::Init()
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

void IntroState::UnInit()
{
	// Keep everything loaded/allocated is also an option
	/*
	TTF_CloseFont( font );
	SDL_DestroyTexture( image );
	SDL_DestroyTexture( blendedText );
	font = nullptr;
	image = nullptr;
	blendedText = nullptr;
	*/
}

void IntroState::ReadJSON(const string & path)
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

void IntroState::WriteJSON(const string & path)
{
	json dataJson{
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

void IntroState::SaveFileDialog()
{
	nfdchar_t *outPath = nullptr;
	nfdresult_t result = NFD_SaveDialog( nullptr, nullptr, &outPath );

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

void IntroState::OpenFileDialog()
{
	nfdchar_t *outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		ReadJSON(outPath);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		println("Canceled by user. Load standard-map");
		//readJson( BasePath"asset/map.json" );
	}
	else{
		println("Error: {}", NFD_GetError() );
	}
}

void IntroState::Events( const u32 frame, const u32 totalMSec, const float deltaT )
{
	SDL_PumpEvents();

	Event event;
	while( SDL_PollEvent( &event ) )
	{
#ifdef IMGUI
		const ImGuiIO & io = ImGui::GetIO();
		ImGui_ImplSDL2_ProcessEvent(&event); // Pass events to ImGui
#endif

		if( game.HandleEvent( event ) )
			continue;

		switch( event.type )
		{
			case SDL_KEYDOWN:
				{
#ifdef IMGUI
					if( io.WantCaptureKeyboard ){	continue;	}
#endif
					InputKeyboard(event);
					break;
				}

			case SDL_MOUSEMOTION:
				{
#ifdef IMGUI
					if( io.WantCaptureMouse ){	continue;	}
#endif
					/// Update current mouse position
					mousepos_.x  = event.motion.x / (scaled_size_);
					mousepos_.y  = event.motion.y / (scaled_size_);
					break;
				}

			case SDL_MOUSEBUTTONDOWN:
				{
#ifdef IMGUI
					if( io.WantCaptureMouse ){	continue;	}
#endif

					if(event.button.button == SDL_BUTTON_LEFT)
					{
						// if mouse is inside LowerPanel, select tile on current mouse pos
						if((mousepos_.y * scaled_size_  >= lower_panel_.y) &&	isAtlasVisible())
						{
							selected_pos_.x = mousepos_.x * scaled_size_;
							selected_pos_.y = mousepos_.y * scaled_size_ - lower_panel_.y;
						}
						// if mouse is NOT in LowerPanel, place last Selected Tile on current Position.
						if((mousepos_.y * scaled_size_ < lower_panel_.y) || !isAtlasVisible()	)
						{
							const Point unscaled_point = {selected_pos_.x / scaled_size_, selected_pos_.y / scaled_size_};
							const int selected_type = static_cast<u16>(pointToInt(unscaled_point, tset_size_array_[tileset_id_].x  / TileSize));
							const int dst_pos = pointToInt(mousepos_, map_header_.cols);
							map_data_.tiles[layer_id_][dst_pos].type = selected_type;
							map_data_.tiles[layer_id_][dst_pos].asset_id = tileset_id_;
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
			default:
				break;
		}}}

void IntroState::InputKeyboard(SDL_Event event)
{
	const Keysym & what_key = event.key.keysym;
	if( what_key.scancode == SDL_SCANCODE_F5 && event.key.repeat == 0 ){
		SaveFileDialog();
	}
	else if(what_key.scancode == SDL_SCANCODE_UP){
		if(camera_map_.y > CameraSpeed){
			camera_map_.y -= CameraSpeed;
		}
	}
	else if(what_key.scancode == SDL_SCANCODE_DOWN){
		if(camera_map_.y < WindowSize.y-CameraSpeed)
		{
			camera_map_.y += CameraSpeed;
		}
	}
	else if(what_key.scancode == SDL_SCANCODE_LEFT){
		if(camera_map_.x > CameraSpeed)
		{
			camera_map_.x -= CameraSpeed;
		}
	}
	else if(what_key.scancode == SDL_SCANCODE_RIGHT){
		if(camera_map_.x > WindowSize.x-CameraSpeed){
			camera_map_.x -= CameraSpeed;
		}
	}
	else if( what_key.scancode == SDL_SCANCODE_F2 ){
		if(isAtlasVisible()){
			atlas_open_ = false;
		}else{
			atlas_open_ = true;
		}
	}
	else if( what_key.scancode == SDL_SCANCODE_ESCAPE ){
		game.SetNextState( 0 );
	}
}

void IntroState::Update( const u32 frame, const u32 totalMSec, const float deltaT )
{

}

void IntroState::Render( const u32 frame, const u32 totalMSec, const float deltaT )
{
	for(const auto & lay : map_data_.tiles)
	{
		for(int tile = 0; tile < lay.size(); tile++)
		{
			assert(lay[tile].type <= EmptyTileVal);
			if(lay[tile].type != EmptyTileVal){
				// Calculate position from type in TileSet
				const Point tsetCoords = intToPoint(lay[tile].type, tset_size_array_[lay[tile].asset_id].x / TileSize);
				const Rect srcRect
				{
					tsetCoords.x * map_header_.tilesize,
					tsetCoords.y * map_header_.tilesize,
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

	if(atlas_open_){
		RenderAtlas();
	}
	RenderGUI();

}

void IntroState::RenderAtlas() const
{
	assert(WindowSize.x > tset_size_array_[tileset_id_].x);
	assert(WindowSize.y > tset_size_array_[tileset_id_].y);

		const int tileNumb = (tset_size_array_[tileset_id_].x * tset_size_array_[tileset_id_].y) / TileSize;
		for(int i = 0; i < tileNumb; i++)
		{
			const Point relCoords = intToPoint(i , tset_size_array_[tileset_id_].x / map_header_.tilesize);
			const Rect srcRect =
			{
				relCoords.x * map_header_.tilesize,
				relCoords.y * map_header_.tilesize,
				map_header_.tilesize, map_header_.tilesize
			};
			const Rect dstRect =
			{
				relCoords.x * scaled_size_,
			   lower_panel_.y + relCoords.y * scaled_size_,
			   scaled_size_, scaled_size_
		   };

			SDL_RenderCopy(render, map_data_.tileSets[tileset_id_].get(), &srcRect, &dstRect);

			// marker for selected tile in atlas
			const Rect mouse_srcRect = {0,0,scaled_size_,scaled_size_};
			const Rect dst_rect = {selected_pos_.x, selected_pos_.y + lower_panel_.y, scaled_size_, scaled_size_};
			SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &dst_rect );

		}
}

void IntroState::RenderGUI()
{
	// Render actual MousePos;
	const Rect mouse_srcRect =	{0, 0, scaled_size_, scaled_size_	};
	const Rect mouse_dstRect =
	{
		mousepos_.x * scaled_size_,
		mousepos_.y * scaled_size_,
		scaled_size_, scaled_size_
	};
	SDL_RenderCopy(render, gui_texture_.get(), &mouse_srcRect, &mouse_dstRect);

#ifdef IMGUI

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	constexpr int MaxSize = 650;
	constexpr int MinSize = 0;
	if(game.imgui_window_active)
	{
		ImGui::Begin("ImGUI window", &game.imgui_window_active);
		ImGui::Checkbox("Show Atlas", &atlas_open_);

		// Control panel sizes
		ImGui::SliderInt("Atlas height", &lower_panel_.h, MinSize, MaxSize);
		lower_panel_.y = ((MaxSize - lower_panel_.h) / scaled_size_) * scaled_size_;
		upper_panel_.h = WindowSize.y - lower_panel_.h;

		// Control current active Layer
		int slider_layer = layer_id_;
		ImGui::SliderInt("active layer:", &slider_layer, 0, LayerNumb-1);
		layer_id_ = static_cast<u8>(slider_layer);

		ImGui::End();
		ImGui::Render();
		SDL_SetRenderDrawColor(render, static_cast <u8>(0.45f * 255), static_cast <u8>(0.55f * 255), static_cast <u8>(0.60f * 255), static_cast <u8>(1.00f * 255));

		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

	}
#endif

}