#include "gamebase.h"

#include "example/examplegame.h"
#include <imgui.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_impl_sdl2.h>
using namespace BzlGame;

Game::Game( const char * windowTitle, const Point windowSize, const bool vSync )
{
	if( SDL_Init( SDL_INIT_EVERYTHING ) )
	{
		print( stderr, "SDL_Init failed: {}\n", SDL_GetError() );
		exit( 1 );
	}

	if( TTF_Init() )
	{
		print( stderr, "TTF_Init failed: {}\n", TTF_GetError() );
		exit( 2 );
	}

	constexpr IMG_InitFlags imgFlags = (IMG_InitFlags) (IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
	if( IMG_Init( imgFlags ) != imgFlags )
	{
		print( stderr, "IMG_Init failed: {}\n", IMG_GetError() );
		exit( 3 );
	}

	constexpr MIX_InitFlags mixFlags = (MIX_InitFlags) (MIX_INIT_MP3 | MIX_INIT_OGG);
	if( Mix_Init( mixFlags ) != mixFlags )
	{
		print( stderr, "Mix_Init failed: {}\n", Mix_GetError() );
		exit( 4 );
	}

	if( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024 ) < 0 )
	{
		print( stderr, "Mix_OpenAudio failed: {}\n", Mix_GetError() );
		exit( 5 );
	}

	constexpr SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

	window = SDL_CreateWindow(
		windowTitle,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowSize.x,
		windowSize.y,
		flags );

	if( window == nullptr )
	{
		print( stderr, "Window could not be created: {}\n", SDL_GetError() );
		exit( 6 );
	}

	render = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
		| (vSync
		   ? SDL_RENDERER_PRESENTVSYNC
		   : 0) );

	if( render == nullptr )
	{
		print( stderr, "Renderer could not be created: {}\n", SDL_GetError() );
		exit( 7 );
	}

	allStates.reserve( 10 );
	std::ranges::fill(allStates, nullptr );

#ifdef IMGUI
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

	// Setup Dear ImGui style
	ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, render);
	ImGui_ImplSDLRenderer2_Init(render);
#endif

}

Game::~Game()
{
	for( GameState * state : allStates )
		delete state;

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	if( render != nullptr )
		SDL_DestroyRenderer( render );

	if( window != nullptr )
		SDL_DestroyWindow( window );

	IMG_Quit();

	if( TTF_WasInit() )
		TTF_Quit();

	if( SDL_WasInit( 0 ) )
		SDL_Quit();

}

bool Game::HandleEvent( const Event event )
{
	switch( event.type )
	{
		case SDL_QUIT:
			isRunning = false;
			return true;
			break;

		case SDL_KEYDOWN:
		{

			auto & key_event = event.key;
			Keysym what_key = key_event.keysym;

			if( what_key.mod & KMOD_ALT &&
			    what_key.scancode == SDL_SCANCODE_F4 )
			{
				Event next_event = { .type = SDL_QUIT };
				SDL_PushEvent( &next_event );
				return true;
			}
			break;
		}
		default:
			break;
	}
	return false;
}

int Game::Run()
{
	SDL_assert( nextStateIdx >= 0 );

	Duration deltaT = Duration::zero();
	Duration deltaTNeeded = Duration::zero();   // How much time was really necessary
	TimePoint start;

	while( IsRunning() )
	{
		start = Clock::now();

		const float deltaTF			= std::chrono::duration<float>( deltaT ).count();
		const float deltaTFNeeded	= std::chrono::duration<float>( deltaTNeeded ).count();

		OutputPerformanceInfo( start, deltaTNeeded );

		ActivateNextState();

		// The difference to last frame is usually 16-17 at 60Hz, 10 at 100Hz, 8-9 at 120Hz, 6-*7* at 144Hz
		const u32 totalMSec = SDL_GetTicks();

		SDL_GetWindowSize(window, &windowSize.x, &windowSize.y);


		currentState->Events( frame, totalMSec, deltaTF );

		currentState->Update( frame, totalMSec, deltaTF );

		currentState->Render( frame, totalMSec, deltaTFNeeded );
		SDL_RenderPresent( render );

		const Color clear = currentState->GetClearColor();
		if( clear.a != SDL_ALPHA_TRANSPARENT)
		{
			SDL_SetRenderDrawColor( render, clear.r, clear.g, clear.b, clear.a );
		}
		SDL_RenderClear(render);


		deltaTNeeded = Clock::now() - start;

		if( currentState->IsFPSLimited() )
		{
			using namespace std::chrono_literals;

			const Duration dur = std::max( Duration::zero(), 16ms - deltaTNeeded );
			const u32 ms = static_cast<u32>( std::chrono::duration_cast<std::chrono::milliseconds>( dur ).count() );
			SDL_Delay( ms );
			deltaT = Clock::now() - start;
		}
		else
		{
			deltaT = deltaTNeeded;
		}

		++frame;
	}
	return 0;
}

void Game::ActivateNextState()
{
	if( nextStateIdx != currentStateIdx
	    && nextStateIdx != -1 )
	{
		// Load the state or die
		if( nextStateIdx >= static_cast<int>(allStates.size()) || allStates[nextStateIdx] == nullptr )
		{
			print( stderr, "Activated out of range or nullptr state with the index: {}\n", nextStateIdx );
			exit( 11 );
		}

		if( currentState != nullptr )
			currentState->UnInit();

		currentStateIdx = nextStateIdx;
		currentState = allStates[currentStateIdx];
		currentState->Init();
	}
}

float Game::AverageMSecPerFrame() const
{
	const u32 passedFrames = frame - lastPerfInfoFrame + 1;
	return std::chrono::duration<float>( accumulatedNeeded / passedFrames ).count() * 1000.0f;
}

void Game::ResetPerformanceInfo( const TimePoint current )
{
	lastPerfInfoFrame = frame;
	lastPerfInfoTime = current;
	accumulatedNeeded = Duration::zero();
}

void Game::OutputPerformanceInfo( const TimePoint current, const Duration needed )
{
	using namespace std::chrono_literals;

	accumulatedNeeded += needed;

	const Duration passedTime = current - lastPerfInfoTime;

	switch( perfDrawMode )
	{
		case PerformanceDrawMode::Title:
			if( passedTime > 250ms )
			{
				const std::string avgms = format( "{}ms", AverageMSecPerFrame() );
				SDL_SetWindowTitle( window, avgms.c_str() );
				ResetPerformanceInfo( current );
			}
			break;

		case PerformanceDrawMode::OStream:
			if( passedTime > 1000ms )
			{
				print( "{}ms", AverageMSecPerFrame() );
				ResetPerformanceInfo( current );
			}
			break;

		case PerformanceDrawMode::None:
		default: break;
	}
}

