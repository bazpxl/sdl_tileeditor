

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

	if( !image )
	{
		image = IMG_LoadTexture( render, BasePath "asset/graphic/testSet.png" );
		if( !image )
			print( stderr, "IMG_LoadTexture failed: {}\n", IMG_GetError() );
	}

	/// TEST reading & writing with JSON
	/// also conversions from Map-to-Json and visa-vie
	{
		// file-to-json
		json j = readJson( BasePath"asset/map.json" );

		// json-to-map
		map = jsonToMap(j, render);

		// map-to-json
		json jf = serializeToJson( map );

		// json-to-file
		writeJson(	jf,	BasePath"asset/map.json"	);

		DebugOnly(		println("{}", jf.dump(0));		)
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
					println("ACTION: F3");
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
	const auto & [x, y] = game.GetWindowSize();

	{
		const Rect dst_rect { 0, 0, x, y };
		SDL_RenderCopy( render, image, EntireRect, &dst_rect /* same result as EntireRect */ );
	}

}
