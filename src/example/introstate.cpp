

#include "examplegame.h"

void IntroState::Init()
{
	if( !font )
	{
		font = TTF_OpenFont( BasePath "asset/font/RobotoSlab-Bold.ttf", 24 );
		TTF_SetFontHinting( font, TTF_HINTING_LIGHT );
		if( !font )
			print( stderr, "TTF_OpenFont failed: {}\n", TTF_GetError() );
	}

	cameraPosition = {0,0};
	cameraView = {0,0, 1280,960};

	// Open file with native file dialog

	nfdchar_t *outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog( nullptr, nullptr, &outPath );

	if ( result == NFD_OKAY )
	{
		DebugOnly(
			println("Load file from: {}", outPath);
		)
		readJson(outPath,mHeader,mData,render);
		free(outPath);
	}
	else if ( result == NFD_CANCEL )
	{
		println("Canceled by user. Load standard-map");
		readJson( BasePath"asset/map.json",mHeader, mData, render );
	}
	else
	{
		println("Error: {}", NFD_GetError() );
		readJson( BasePath"asset/map.json",mHeader, mData, render );
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

void IntroState::Events( const u32 frame, const u32 totalMSec, const float deltaT )
{
	SDL_PumpEvents();

	Event event;
	while( SDL_PollEvent( &event ) )
	{
		if( game.HandleEvent( event ) )
			continue;

		switch( event.type )
		{
			case SDL_KEYDOWN:
			{
				const Keysym & what_key = event.key.keysym;

				if( what_key.scancode == SDL_SCANCODE_F3 && event.key.repeat == 0 )
				{
					nfdchar_t *outPath = nullptr;
					nfdresult_t result = NFD_SaveDialog( nullptr, nullptr, &outPath );

					if ( result == NFD_OKAY )
					{
						DebugOnly
						(
							print("Save file to: {}", outPath);
						)
						writeJson(outPath,mHeader,mData);
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
				else if( what_key.scancode == SDL_SCANCODE_F9 )
				{
					// crash/shutdown, since State #6 does not exist
					game.SetNextState( 99 );
				}
				else if( what_key.scancode == SDL_SCANCODE_ESCAPE )
				{
					game.SetNextState( 0 );
				}

				break;
			}

			case SDL_MOUSEBUTTONDOWN:
				println("ACTION: MouseButtonDown");
				break;

			default:
				break;
		}
	}
}

void IntroState::Update( const u32 frame, const u32 totalMSec, const float deltaT )
{
}

void IntroState::Render( const u32 frame, const u32 totalMSec, const float deltaT )
{

	int rows = mHeader.rows;
	int tileSize = mHeader.tileSize;
	Rect tileRect{0,0,tileSize,tileSize};

	int set_width, set_height;
	if(SDL_QueryTexture(mData.tileSets[0].get(), nullptr, nullptr,&set_width,&set_height ) != 0)[[unlikely]]
	{
		println("Error: {}", SDL_GetError());
	}

	{
		for(const auto & layer : mData.tiles)
		{
			for(int tile = 0; tile < layer.size(); tile++)
			{
				const u8 id_	= layer[tile].assetID;
				const u16 type_ = layer[tile].type;

				if(id_ != 0)[[unlikely]]
				{
					SDL_QueryTexture(mData.tileSets[id_].get(), nullptr, nullptr, &set_width,&set_height );
				}

				const Point srcPos = intToPoint(type_,set_width);
				const Rect srcRect = {srcPos.x,srcPos.y,tileSize,tileSize};

				const Point pos = intToPoint(tile,rows);
				const Rect dstRect
				{
					(pos.x * tileSize) - cameraPosition.x, // Adjust for camera position
					(pos.y * tileSize) - cameraPosition.y, // Adjust for camera position
					tileSize, tileSize
				};

				if (isTileInCameraView(dstRect)){
					SDL_RenderCopy(render, mData.tileSets[id_].get(), &srcRect, &dstRect);
				}
			}
		}

	}

}
