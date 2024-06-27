#pragma once

#include <global.h>

#include <gamebase.h>
#include "recthelper.h"
#include <tilemap.h>

class ExampleGame;
class IntroState;
using namespace BzlGame;

class ExampleGame final : public Game
{
public:
	ExampleGame();

	bool HandleEvent( const Event event ) override;
};

class IntroState : public GameState
{
protected:
	Font    * font              = nullptr;
	Texture * image             = nullptr;
	//Texture * blendedText       = nullptr;

	Point cameraPosition {0,0}; // Camera position
	Rect cameraView;				// Camera view rectangle

	bool isTileInCameraView(const Rect& dstRect) {
		return SDL_HasIntersection(&dstRect, &cameraView);
	}



public:
	MapHeader mHeader;
	MapData mData;

	// ctor
	using GameState::GameState;

	void Init() override;
	void UnInit() override;

	void Events( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Update( const u32 frame, const u32 totalMSec, const float deltaT ) override;
	void Render( const u32 frame, const u32 totalMSec, const float deltaT ) override;
};

