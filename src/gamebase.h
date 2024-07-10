#pragma once

#include <global.h>

using namespace BzlGame;

class Game;
class GameState;

class Game
{
protected:
	Window   * window;
	Renderer * render;

	GameState * currentState = nullptr;
	Vector<GameState *> allStates;

	u32  frame     = 0;

	int currentStateIdx = -1;
	int nextStateIdx    = -1;

	Point windowSize {WindowSize};
	bool isRunning = true;

public:
#ifdef IMGUI
	bool imgui_window_active = true;
#endif

	[[nodiscard]]       bool    IsRunning()     const { return isRunning;   }
	[[nodiscard]]       Point & GetWindowSize()       { return windowSize;  }
	[[nodiscard]] const Point & GetWindowSize() const { return windowSize;  }
	[[nodiscard]]		Window* GetWindow()			  { return window;		}

	explicit Game(
		const char * windowTitle = "SDL Game",
		const Point  windowSize  = WindowSize,
		const bool   vSync       = true );
	Game(              const Game &  ) = delete;
	Game(                    Game && ) = delete;
	Game &  operator=( const Game &  ) = delete;
	Game && operator=(       Game && ) = delete;
	virtual ~Game();

	virtual bool HandleEvent( const Event event );
	virtual int Run();

	virtual void SetNextState( int index ) { nextStateIdx = index; }

protected:
	virtual void ActivateNextState();


	/// Performance
public:
	// All output msec per frame, see: http://renderingpipeline.com/2013/02/fps-vs-msecframe/
	enum class PerformanceDrawMode
	{
		None,
		Title,      // averaged and only updated every 250ms
		OStream,    // averaged and only updated every 1000ms
	};

	void SetPerfDrawMode( PerformanceDrawMode mode ) noexcept { perfDrawMode = mode; }

protected:
	PerformanceDrawMode perfDrawMode = PerformanceDrawMode::None;
	u32 lastPerfInfoFrame = 0;
	TimePoint lastPerfInfoTime = Clock::now();
	Duration accumulatedNeeded = Duration::zero();

	[[nodiscard]]
	float AverageMSecPerFrame() const;
	void ResetPerformanceInfo( const TimePoint current );
	void OutputPerformanceInfo( const TimePoint current, const Duration needed );
};

class GameState
{
protected:
	Game     & game;
	Renderer * render;

public:
	[[nodiscard]] virtual bool  IsFPSLimited()  const { return true; }
	[[nodiscard]] virtual Color GetClearColor() const { return Color { 0, 0, 0, SDL_ALPHA_OPAQUE }; }

	explicit GameState( Game && game, Renderer * render ) = delete; // prevent taking an rvalue
	explicit GameState( Game &  game, Renderer * render )
		: game( game ),
		  render( render )
	{}
	GameState(              const GameState &  ) = delete;
	GameState(                    GameState && ) = delete;
	GameState &  operator=( const GameState &  ) = delete;
	GameState && operator=(       GameState && ) = delete;
    virtual ~GameState() = default;

	virtual void Init() {}
	virtual void UnInit() {}

	virtual void Events( const u32 frame, const u32 totalMSec, const float deltaT ) = 0;
	virtual void Update( const u32 frame, const u32 totalMSec, const float deltaT ) = 0;
	virtual void Render( const u32 frame, const u32 totalMSec, const float deltaT ) = 0;
};
