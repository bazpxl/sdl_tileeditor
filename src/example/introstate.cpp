

#include "examplegame.h"

#include <imgui.h>
#include <../imgui/imgui_impl_sdl2.h>
#include <../imgui/imgui_impl_sdlrenderer2.h>

void IntroState::Init()
{
	if( !pfont )
	{
		pfont = TTF_OpenFont( BasePath "asset/font/RobotoSlab-Bold.ttf", 24 );
		TTF_SetFontHinting( pfont, TTF_HINTING_LIGHT );
		if( !pfont )
			print( stderr, "TTF_OpenFont failed: {}\n", TTF_GetError() );
	}

	gui_texture = CreateSharedTexture(render, BasePath"asset/graphic/editorGUI.png");

	OpenFileDialog();

	for(int i = 0; i < m_data.tileSets.size(); i++)
	{
		SDL_QueryTexture(m_data.tileSets[i].get(), nullptr, nullptr,&tset_size_array[i].x, &tset_size_array[i].y );
		tset_size_array[i].x = tset_size_array[i].x;
		tset_size_array[i].y = tset_size_array[i].y;
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

void IntroState::SaveFileDialog()
{
	nfdchar_t *outPath = nullptr;
	nfdresult_t result = NFD_SaveDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		DebugOnly
		(
			print("Save file to: {}", outPath);
		)
		writeJson(outPath,m_header,m_data);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		DebugOnly
		(
			println("Canceled file saving");
		)
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
		readJson(outPath,m_header,m_data,render);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		println("Canceled by user. Load standard-map");
		readJson( BasePath"asset/map.json",m_header, m_data, render );
	}
	else
	{
		println("Error: {}", NFD_GetError() );
		readJson( BasePath"asset/map.json",m_header, m_data, render );
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
				const Keysym & what_key = event.key.keysym;

				if( what_key.scancode == SDL_SCANCODE_F5 && event.key.repeat == 0 )
				{
					SaveFileDialog();
				}
				else if(what_key.scancode == SDL_SCANCODE_UP)
				{
					if(camera_map.y > CameraSpeed)
					{
						camera_map.y -= CameraSpeed;
					}
				}
				else if(what_key.scancode == SDL_SCANCODE_DOWN)
				{
					if(camera_map.y < WindowSize.y-CameraSpeed)
					{
						camera_map.y += CameraSpeed;
					}

				}
				else if(what_key.scancode == SDL_SCANCODE_LEFT)
				{
					if(camera_map.x > CameraSpeed)
					{
						camera_map.x -= CameraSpeed;
					}
				}
				else if(what_key.scancode == SDL_SCANCODE_RIGHT)
				{
					if(camera_map.x > WindowSize.x-CameraSpeed)
					{
						camera_map.x -= CameraSpeed;
					}
				}

				else if( what_key.scancode == SDL_SCANCODE_F2 )
				{
					if(isAtlasVisible())
					{
						atlas_open = false;
					}else
					{
						atlas_open = true;
					}
				}
				else if( what_key.scancode == SDL_SCANCODE_ESCAPE )
				{
					game.SetNextState( 0 );
				}
				break;
			}

			case SDL_MOUSEMOTION:
#ifdef IMGUI
				if( io.WantCaptureMouse ){	continue;	}
#endif
				/// Update current mouse position
				mouseposition.x  = event.motion.x / (scaledTileSize);
				mouseposition.y  = event.motion.y / (scaledTileSize);
			break;

			case SDL_MOUSEBUTTONDOWN:
#ifdef IMGUI
				if( io.WantCaptureMouse ){	continue;	}
#endif

					if(event.button.button == SDL_BUTTON_LEFT)
					{
						// if mouse is inside LowerPanel, select tile on current mouse pos
						if((mouseposition.y * scaledTileSize  >= LowerPanel.y) &&	isAtlasVisible())
						{
							selectedtile.x = mouseposition.x * scaledTileSize;
							selectedtile.y = mouseposition.y * scaledTileSize - LowerPanel.y;
						}
						// if mouse is NOT in LowerPanel, place last Selected Tile on current Position.
						if((mouseposition.y * scaledTileSize < LowerPanel.y) || !isAtlasVisible()	)
						{
							const Point unscaled_point = {selectedtile.x / scaledTileSize, selectedtile.y / scaledTileSize};
							const int selected_type = pointToInt(unscaled_point, tset_size_array[tileset_id].x  / TileSize);
							const int dst_pos = pointToInt(mouseposition, m_header.cols);
							m_data.tiles[active_layer_id][dst_pos].type = selected_type;
							m_data.tiles[active_layer_id][dst_pos].assetID = tileset_id;
							//println("m_data.tiles[{}] {}  {}",active_layer_id, m_data.tiles[active_layer_id][dst_pos].type, m_data.tiles[active_layer_id][dst_pos].assetID);
						}
					}else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						// if mouse is in valid Panel, click to remove tile type and replace it with EmptyTileVal.
						if((mouseposition.y * scaledTileSize < LowerPanel.y) || !isAtlasVisible()	)
						{
							const int dst_pos = pointToInt(mouseposition, m_header.cols);
							m_data.tiles[active_layer_id][dst_pos].type = EmptyTileVal;
						}
					}
				break;
			default:
				break;
		}}}

void IntroState::Update( const u32 frame, const u32 totalMSec, const float deltaT )
{

}

void IntroState::Render( const u32 frame, const u32 totalMSec, const float deltaT )
{

	for(const auto & lay : m_data.tiles)
	{
		for(int tile = 0; tile < lay.size(); tile++)
		{
			assert(lay[tile].type >= 0 && lay[tile].type <= EmptyTileVal);
			if(lay[tile].type != EmptyTileVal){
				// Calculate position from type in TileSet
				const Point tsetCoords = intToPoint(lay[tile].type, tset_size_array[lay[tile].assetID].x / TileSize);
				const Rect srcRect
				{
					tsetCoords.x * m_header.tileSize,
					tsetCoords.y * m_header.tileSize,
					m_header.tileSize, m_header.tileSize
				};

				// Calculate destination on Map
				const Point dstCoords = intToPoint(tile,m_header.rows);
				const Rect dstRect
				{
					dstCoords.x *scaledTileSize - camera_map.x  ,
					dstCoords.y *scaledTileSize - camera_map.y  ,
					scaledTileSize, scaledTileSize
				};

				if(atlas_open)
				{
					if(dstRect.y+TileSize < LowerPanel.y)
					{
						SDL_RenderCopy(render, m_data.tileSets[lay[tile].assetID].get(), &srcRect, &dstRect);
					}
				}else
				{
					SDL_RenderCopy(render, m_data.tileSets[lay[tile].assetID].get(), &srcRect, &dstRect);
				}
			}
		}
	}

	if(atlas_open){
		RenderAtlas();
	}
	RenderGUI();

}

void IntroState::RenderAtlas() const
{
	assert(WindowSize.x > tset_size_array[tileset_id].x);
	assert(WindowSize.y > tset_size_array[tileset_id].y);

		const int tileNumb = (tset_size_array[tileset_id].x * tset_size_array[tileset_id].y) / TileSize;
		for(int i = 0; i < tileNumb; i++)
		{
			const Point relCoords = intToPoint(i , tset_size_array[tileset_id].x / m_header.tileSize);
			const Rect srcRect =
			{
				relCoords.x * m_header.tileSize,
				relCoords.y * m_header.tileSize,
				m_header.tileSize, m_header.tileSize
			};
			const Rect dstRect =
			{
				relCoords.x * scaledTileSize,
			   LowerPanel.y + relCoords.y * scaledTileSize,
			   scaledTileSize, scaledTileSize
		   };

			SDL_RenderCopy(render, m_data.tileSets[tileset_id].get(), &srcRect, &dstRect);

			// marker for selected tile in atlas
			const Rect mouse_srcRect = {0,0,scaledTileSize,scaledTileSize};
			const Rect dst_rect = {selectedtile.x, selectedtile.y + LowerPanel.y, scaledTileSize, scaledTileSize};
			SDL_RenderCopy(render, gui_texture.get(), &mouse_srcRect, &dst_rect );

		}
}

void IntroState::RenderGUI()
{
	// Render actual MousePos;
	const Rect mouse_srcRect =	{0, 0, scaledTileSize, scaledTileSize	};
	const Rect mouse_dstRect =
	{
		mouseposition.x * scaledTileSize,
		mouseposition.y * scaledTileSize,
		scaledTileSize, scaledTileSize
	};
	SDL_RenderCopy(render, gui_texture.get(), &mouse_srcRect, &mouse_dstRect);

#ifdef IMGUI

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	constexpr int MaxSize = 650;
	constexpr int MinSize = 50;
	if(game.imgui_window_active)
	{
		ImGui::Begin("ImGUI window", &game.imgui_window_active);
		ImGui::Checkbox("Show Atlas", &atlas_open);

		// Control panel sizes
		ImGui::SliderInt("Atlas height", &LowerPanel.h, MinSize, MaxSize);
		LowerPanel.y = ((MaxSize - LowerPanel.h) / scaledTileSize) * scaledTileSize;
		UpperPanel.h = WindowSize.y - LowerPanel.h;

		// Control current active Layer
		int slider_layer = active_layer_id;
		ImGui::SliderInt("active layer:", &slider_layer, 0, LayerNumb);
		active_layer_id = static_cast<u8>(slider_layer);

		ImGui::End();
		ImGui::Render();
		SDL_SetRenderDrawColor(render, (Uint8)(0.45f * 255), (Uint8)(0.55f * 255), (Uint8)(0.60f * 255), (Uint8)(1.00f * 255));

		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), render);

	}
#endif

}