#include <QJsonArray>
#include <QJsonObject>
#include <catch.hpp>

#include "src/state/robot_map.h"

using namespace Valeronoi::state;

TEST_CASE("RobotMap basic functionality", "[state]") {
  RobotMap map;

  SECTION("Tight cropping without size") {
    QJsonObject mapJson;
    mapJson.insert("__class", "ValetudoMap");
    QJsonObject metaData;
    metaData.insert("version", 2);
    mapJson.insert("metaData", metaData);
    mapJson.insert("pixelSize", 10);

    QJsonArray layers;
    QJsonObject floor;
    floor.insert("type", "floor");
    QJsonArray compressedPixels;
    // x=100, y=100, count=1 -> pixels [1000, 1000] to [1010, 1010]
    compressedPixels.append(100);
    compressedPixels.append(100);
    compressedPixels.append(1);
    floor.insert("compressedPixels", compressedPixels);
    layers.append(floor);
    mapJson.insert("layers", layers);

    map.update_map_json(mapJson);
    REQUIRE(map.is_valid());

    const auto& m = map.get_map();
    // If it's tightly cropped, size should be pixel_size (10) and crop_x should
    // be 1000. Current code: min_x = min(0, 1000) = 0. max_x = max(0, 1010) =
    // 1010. So size_x would be 1010.
    CHECK(m.size_x == 10);
    CHECK(m.size_y == 10);
    CHECK(m.crop_x == 1000);
    CHECK(m.crop_y == 1000);
  }
}
