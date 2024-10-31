/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2024 Christian Friedrich Coors <me@ccoors.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <array>
#include <catch.hpp>
#include <vector>

#include "src/util/colormap.h"

void assert_color(const Valeronoi::util::RGBColorMap &m, double pos, double e1,
                  double e2, double e3) {
  const auto c = m.get_color(pos);
  CHECK(c[0] == Approx(e1).margin(0.001));
  CHECK(c[1] == Approx(e2).margin(0.001));
  CHECK(c[2] == Approx(e3).margin(0.001));
}

TEST_CASE("ColorMap without colors throws", "[colormap]") {
  std::vector<std::array<double, 3>> colors;
  CHECK_THROWS(Valeronoi::util::RGBColorMap("Foo", colors));
}

TEST_CASE("ColorMap works with single color", "[colormap]") {
  std::vector<std::array<double, 3>> colors;
  CHECK_THROWS(Valeronoi::util::RGBColorMap("Foo", colors));
  colors.push_back({0.0, 1.0, 0.5});
  auto const c = Valeronoi::util::RGBColorMap("Foo", colors);
  CHECK(c.name() == "Foo");
  assert_color(c, 0.0, 0.0, 1.0, 0.5);
  assert_color(c, 1.0, 0.0, 1.0, 0.5);
  assert_color(c, -1.0, 0.0, 1.0, 0.5);
  assert_color(c, 2.0, 0.0, 1.0, 0.5);
}

TEST_CASE("ColorMap works with two colors", "[colormap]") {
  std::vector<std::array<double, 3>> colors;
  CHECK_THROWS(Valeronoi::util::RGBColorMap("Foo", colors));
  colors.push_back({0.0, 1.0, 0.5});
  colors.push_back({1.0, 0.5, 0.5});
  auto const c = Valeronoi::util::RGBColorMap("Foo", colors);
  assert_color(c, 0.0, 0.0, 1.0, 0.5);
  assert_color(c, 0.5, 0.5, 0.75, 0.5);
  assert_color(c, 1.0, 1.0, 0.5, 0.5);
  assert_color(c, -1.0, 0.0, 1.0, 0.5);
  assert_color(c, 2.0, 1.0, 0.5, 0.5);
}

TEST_CASE("ColorMap works with multiple colors", "[colormap]") {
  std::vector<std::array<double, 3>> colors;
  CHECK_THROWS(Valeronoi::util::RGBColorMap("Foo", colors));
  colors.push_back({0.0, 1.0, 0.5});
  colors.push_back({0.5, 0.5, 0.5});
  colors.push_back({0.6, 1.0, 0.5});
  auto const c = Valeronoi::util::RGBColorMap("Foo", colors);
  assert_color(c, 0.0, 0.0, 1.0, 0.5);
  assert_color(c, 0.000000001, 0.0, 1.0, 0.5);
  assert_color(c, 0.25, 0.25, 0.75, 0.5);
  assert_color(c, 0.5, 0.5, 0.5, 0.5);
  assert_color(c, 0.75, 0.55, 0.75, 0.5);
  assert_color(c, 0.999999999, 0.6, 1.0, 0.5);
  assert_color(c, 1.0, 0.6, 1.0, 0.5);
}
