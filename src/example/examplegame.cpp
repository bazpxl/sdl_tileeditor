
#include <imgui.h>

#include "examplegame.h"

ExampleGame::ExampleGame()
	: Game( "BZ_tilemap_editor", WindowSize, true )
{
	// Will be freed in Game dtor
	allStates = {
		new EditorState  ( *this, render )   // 0
	};

	// The virtual call is ok here
	SetNextState( 0 );

	SetPerfDrawMode(PerformanceDrawMode::Title);
}



bool ExampleGame::HandleEvent( const Event event )
{
#ifdef IMGUI
	const ImGuiIO & io = ImGui::GetIO();
	if( io.WantCaptureKeyboard ){	return true;	}
#endif

	// Global Keybindings, will be checked before the ones in GameState
	switch( event.type )
	{
		case SDL_KEYDOWN:
		{
			const Keysym & what_key = event.key.keysym;
			if( what_key.scancode == SDL_SCANCODE_1 )
			{
				// Both Game and GameState can change the State,
				// but it will only be changed after the current frame
				SetNextState( 0 );
				return true;
			}
			break;
		}
		default:
			break;
	}
	return Game::HandleEvent( event );
}
