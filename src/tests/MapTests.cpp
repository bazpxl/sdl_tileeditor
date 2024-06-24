#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "./tilemap.h"

using namespace BzlGame;


TEST_CASE("Testing Tile") {
    Tile tile;

    SUBCASE("Testing default values") {
        CHECK(tile.type == 1);
        CHECK(tile.assetID == 0);
    }

    SUBCASE("Testing setters") {
        tile.type = 2;
        tile.assetID = 1;

        CHECK(tile.type == 2);
        CHECK(tile.assetID == 1);
    }
}

TEST_CASE("Testing Layer") {
    Layer layer;

    SUBCASE("Testing default values") {
        CHECK(layer.tileVec.size() == MAP_COLS * MAP_ROWS);
        CHECK(layer.isVisible == true);
    }

    SUBCASE("Testing setters") {
        layer.tileVec.push_back({2, 1});
        layer.isVisible = false;

        CHECK(layer.tileVec.size() == MAP_COLS * MAP_ROWS + 1);
        CHECK(layer.isVisible == false);
    }
}

TEST_CASE("Testing Map") {
    Map map;

    SUBCASE("Testing default values") {
        CHECK(map.layer.size() == LAYER_NUMB_DEFAULT);
        CHECK(map.tileSets.size() == 0);
        CHECK(map.tileSetPaths.size() == 1);
        CHECK(map.rows == MAP_ROWS);
        CHECK(map.cols == MAP_COLS);
        CHECK(map.tileSize == TILE_SIZE);
    }

    SUBCASE("Testing setters") {
        map.layer.push_back({{}, true});
        map.tileSets.push_back(nullptr);
        map.tileSetPaths.emplace_back("new_path");
        map.rows = 10;
        map.cols = 10;
        map.tileSize = 10;

        CHECK(map.layer.size() == LAYER_NUMB_DEFAULT + 1);
        CHECK(map.tileSets.size() == 1);
        CHECK(map.tileSetPaths.size() == 2);
        CHECK(map.rows == 10);
        CHECK(map.cols == 10);
        CHECK(map.tileSize == 10);
    }
}

TEST_CASE("Testing jsonToMap")
{

    if( SDL_Init( SDL_INIT_EVERYTHING ) ){  print( stderr, "SDL_Init failed: {}\n", SDL_GetError() );}

    constexpr IMG_InitFlags imgFlags = (IMG_InitFlags) (IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP);
    if( IMG_Init( imgFlags ) != imgFlags )  {print( stderr, "IMG_Init failed: {}\n", IMG_GetError() );}

    Window* window = SDL_CreateWindow(
        "windowTitle",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        124,
        72,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

    Renderer* render = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
        | SDL_RENDERER_PRESENTVSYNC);

    json j = readJson();
    Map map = jsonToMap(j, render);

    CHECK(map.layer.size() == LAYER_NUMB_DEFAULT);
    CHECK(map.tileSets.size() == 1);
    CHECK(map.tileSetPaths.size() == 1);
    CHECK(map.rows == MAP_ROWS);
    CHECK(map.cols == MAP_COLS);
    CHECK(map.tileSize == TILE_SIZE);
}

TEST_CASE("Testing serializeToJson")
{
    Map map;
    json j = serializeToJson(map);

    CHECK(j["rows"] == MAP_ROWS);
    CHECK(j["cols"] == MAP_COLS);
    CHECK(j["tileSize"] == TILE_SIZE);
    CHECK(j["layer"].size() == LAYER_NUMB_DEFAULT);
    CHECK(j["tileSetPaths"].size() == 1);
}

TEST_CASE("Testing readJson") {
    json j = readJson();

    CHECK(j["rows"] == MAP_ROWS);
    CHECK(j["cols"] == MAP_COLS);
    CHECK(j["tileSize"] == TILE_SIZE);
    CHECK(j["layer"].size() == LAYER_NUMB_DEFAULT);
    CHECK(j["tileSetPaths"].size() == 1);
}

