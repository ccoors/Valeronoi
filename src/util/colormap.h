#ifndef VALERONOI_UTIL_COLORMAP_H
#define VALERONOI_UTIL_COLORMAP_H

#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace Valeronoi::util {
template <typename T, int components>
class ColorMap {
 public:
  ColorMap(const char *name, std::vector<std::array<T, components>> colors)
      : colormap_name{name}, colormap_colors{std::move(colors)} {
    if (colormap_colors.size() == 0) {
      throw std::runtime_error("Can not initialize ColorMap without colors");
    }
  };

  [[nodiscard]] std::string name() const { return colormap_name; };

  [[nodiscard]] std::array<T, components> get_color(T f) const {
    if (f <= 0) return colormap_colors[0];
    if (f >= 1) return colormap_colors[colormap_colors.size() - 1];
    double place = (colormap_colors.size() - 1) * f;
    double int_part, fract_part;
    fract_part = std::modf(place, &int_part);
    std::size_t index = int_part;
    if (index == colormap_colors.size() - 1) {
      // VERY close to end
      return colormap_colors[colormap_colors.size() - 1];
    }
    assert((index < colormap_colors.size() - 1));

    std::array<T, components> ret{};
    for (std::size_t i = 0; i < components; i++) {
      ret[i] = (1.0 - fract_part) * colormap_colors[index][i];
    }

    for (std::size_t i = 0; i < components; i++) {
      ret[i] += fract_part * colormap_colors[index + 1][i];
    }

    return ret;
  }

 private:
  std::string colormap_name;
  std::vector<std::array<T, components>> colormap_colors;
};

typedef ColorMap<double, 3> RGBColorMap;
typedef std::array<double, 3> RGBColorMapColors;

}  // namespace Valeronoi::util

#endif
