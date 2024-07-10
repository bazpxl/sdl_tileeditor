#pragma once

#define _GLIBC_USE_DEPRECATED 0
#define _GLIBCXX_USE_DEPRECATED 0
#define _LIBCPP_ENABLE_DEPRECATION_WARNINGS 2
#define WIN32_LEAN_AND_MEAN

#include <cmath>
#include <cstdarg>
#include <cstddef>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <fstream>
#include <memory>
//#include <sstream>
#include <string>
#include <vector>

#include <fmt/core.h>   // https://fmt.dev/latest/index.html

#include <nfd.h>        // https://github.com/mlabbe/nativefiledialog/blob/master/README.md

#include <SDL_stdinc.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define IMGUI true

namespace BzlGame
{

	using i8    = std::int8_t;
	using i16   = std::int16_t;
	using i32   = std::int32_t;
	using i64   = std::int64_t;
	using u8    = std::uint8_t;
	using u16   = std::uint16_t;
	using u32   = std::uint32_t;
	using u64   = std::uint64_t;
	using uint  = unsigned int;

	using isize = std::ptrdiff_t;
	using usize = std::size_t;

	using Color     = SDL_Color;
	using Event     = SDL_Event;
	using Point     = SDL_Point;
	using FPoint    = SDL_FPoint;
	using Keysym    = SDL_Keysym;
	using Rect      = SDL_Rect;
	using FRect     = SDL_FRect;
	using Surface   = SDL_Surface;
	using Texture   = SDL_Texture;
	using Renderer  = SDL_Renderer;
	using Window    = SDL_Window;
	using Font      = TTF_Font;
	using Chunk     = Mix_Chunk;
	//using Sound     = Mix_Chunk;
	using Music     = Mix_Music;

	using string = std::string;

	using std::min;
	using std::max;

	template<class T, size_t Size>  using Array  = std::array<T, Size>;
	template<class T>               using Vector = std::vector<T>;
	template<class T>				using SharedPtr = std::shared_ptr<T>;

	using Clock     = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration  = Clock::duration;

	//using std::cout, std::cin, std::cerr, std::endl;
	using fmt::print, fmt::println, fmt::format;

	constexpr u16 TileSize		=	32;
	constexpr Point WindowSize	=  {	1600, 900	};



	struct bzDeleter{
		void operator()(Window *p)    const { SDL_DestroyWindow(p);    }
		void operator()(Renderer *p)  const { SDL_DestroyRenderer(p);  }
		void operator()(Texture *p)   const { SDL_DestroyTexture(p);    }
	};

	inline SharedPtr<Texture>
	CreateSharedTexture(SDL_Renderer *renderer, const char * const file)
	{
		Texture *texture = IMG_LoadTexture(renderer, file);
		if (!texture) throw std::runtime_error("ERROR: IMG_LoadTexture() \n");// handle error or return nullptr
		return {texture, bzDeleter()};
	}

	/// Returns empty Texture with
	inline SharedPtr<Texture>
	CreateSharedTexture(SDL_Renderer *renderer, Point size)
	{
		Texture* texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_TARGET,
			size.x, size.y);
		return {texture, bzDeleter()};
	}

	inline SharedPtr<Renderer>
	CreateSharedRenderer(const SharedPtr<Window>& window, int index, Uint32 flags)
	{
		Renderer* renderer = SDL_CreateRenderer(window.get(), index, flags);
		if(renderer == nullptr) throw std::runtime_error("ERROR: Could not create renderer.\n");
		return {renderer, bzDeleter()};
	}

	inline SharedPtr<Window>
	CreateSharedWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
	{
		Window* window = SDL_CreateWindow(title, x, y, w, h, flags);
		if(!window) throw std::runtime_error("ERROR: Could not create window.\n");
		return {window, bzDeleter()};
	}

}

// _MSC_VER is also true for Clang on Windows, which is fine, but we might need a branch for CLion vs Visual Studio
#ifdef _MSC_VER
//#define BasePath "../../../"    // out/build/${architecture}-${configuration}
#else
//#define BasePath "../"          // cmake-build-${configuration}
#endif

#ifdef FINAL
#define BasePath ""             // Put the binary in the root folder, parallel to assets/
#else
#define BasePath "../../"    //    out/build/${architecture}-${configuration}
                                // OR build/${configuration}-${compiler}/bin
#endif

#if defined( DEBUG )
#define DebugOnly( ... ) __VA_ARGS__
#define IfDebug if constexpr( true )
#else
#define DebugOnly( ... )
#define IfDebug if constexpr( false )
#endif
