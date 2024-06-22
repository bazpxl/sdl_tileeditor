#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "tilemap.h"

using namespace BzlGame;

TEST_CASE("Tile") {
    SUBCASE("Default constructor") {
        Tile tile;
        CHECK(tile.type == 1);
        CHECK(tile.assetID == 0);
    }

    SUBCASE("Constructor with arguments") {
        Tile tile(2, 5);
        CHECK(tile.type == 2);
        CHECK(tile.assetID == 5);
    }
}

TEST_CASE("Layer") {
    SUBCASE("Default constructor") {
        Layer layer;
        CHECK(layer.tileVec.size() == MAP_COLS * MAP_ROWS);
        CHECK(layer.isVisible);
    }

    SUBCASE("Constructor with arguments") {
        Vector<Tile> tileVec = {Tile(1, 2), Tile(3, 4)};
        Layer layer(tileVec, false);
        CHECK(layer.tileVec.size() == 2);
        CHECK_FALSE(layer.isVisible);
    }
}

TEST_CASE("Map") {
    SUBCASE("Default constructor") {
        Map map;
        CHECK(map.layer.size() == LAYER_NUMB_DEFAULT);
        CHECK(map.tileSetPaths.size() == 1);
        CHECK(map.tileSetPaths[0] == BasePath"asset/tSet1.png");
        CHECK(map.rows == MAP_ROWS);
        CHECK(map.cols == MAP_COLS);
        CHECK(map.tileSize == TILE_SIZE);
    }

    SUBCASE("Constructor with arguments") {
        Vector<Layer> layerVec = {Layer(), Layer()};
        Vector<string> tileSetPaths = {BasePath"asset/tSet1.png", BasePath"asset/tSet2.png"};
        Map map(layerVec, tileSetPaths, 10, 20, 30);
        CHECK(map.layer.size() == 2);
        CHECK(map.tileSetPaths.size() == 2);
        CHECK(map.tileSetPaths[0] == BasePath"asset/tSet1.png");
        CHECK(map.tileSetPaths[1] == BasePath"asset/tSet2.png");
        CHECK(map.rows == 10);
        CHECK(map.cols == 20);
        CHECK(map.tileSize == 30);
    }

    SUBCASE("jsonToMap") {
        json js = {
            {"layer", {
                {
                    {"tileVec", {
                        {{"type", 1}, {"assetID", 2}},
                        {{"type", 3}, {"assetID", 4}}
                    }},
                    {"isVisible", true}
                },
                {
                    {"tileVec", {
                        {{"type", 5}, {"assetID", 6}},
                        {{"type", 7}, {"assetID", 8}}
                    }},
                    {"isVisible", false}
                }
            }},
            {"tileSetPaths", {BasePath"asset/tSet1.png", BasePath"asset/tSet2.png"}},
            {"rows", 10},
            {"cols", 20},
            {"tileSize", 30}
        };
        Map map = jsonToMap(js);
        CHECK(map.layer.size() == 2);
        CHECK(map.tileSetPaths.size() == 2);
        CHECK(map.tileSetPaths[0] == BasePath"asset/tSet1.png");
        CHECK(map.tileSetPaths[1] == BasePath"asset/tSet2.png");
        CHECK(map.rows == 10);
        CHECK(map.cols == 20);
        CHECK(map.tileSize == 30);
    }
}
