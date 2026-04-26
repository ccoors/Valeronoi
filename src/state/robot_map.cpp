/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
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
#include "robot_map.h"

#include <algorithm>
#include <limits>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Valeronoi::state {
RobotMap::RobotMap(QObject* parent) : QObject(parent) { reset(); }

void RobotMap::update_map_json(const QString& json) {
  m_error = "";
  m_valid = false;
  if (json == "null") {
    // Edge case, because QJsonDocument::fromJson also returns null if it
    // detects a parsing error
    m_error = tr("No map data");
    emit signal_map_updated();
    return;
  }

  QJsonParseError error;
  auto json_document = QJsonDocument::fromJson(json.toUtf8(), &error);
  if (json_document.isNull()) {
    m_error = error.errorString();
    emit signal_map_updated();
    return;
  }
  if (json_document.isEmpty()) {
    m_error = tr("No map data");
    emit signal_map_updated();
    return;
  }
  update_map_json(json_document.object());
}

void RobotMap::update_map_json(const QJsonObject& json_object) {
  if (json_object["__class"].toString() != "ValetudoMap") {
    m_error = tr("Did not receive ValetudoMap");
    emit signal_map_updated();
    return;
  }

  m_map_version = json_object["metaData"].toObject()["version"].toInt();
  if (m_map_version < 1 || m_map_version > 2) {
    m_error = tr("Unknown map version");
    emit signal_map_updated();
    return;
  }

  m_map_json = json_object;
  generate_map();
  m_valid = true;
  emit signal_map_updated();
}

static void parse_entity_points(const QJsonArray& pixels,
                                std::vector<Point>& points, int* min_x,
                                int* max_x, int* min_y, int* max_y) {
  points.reserve(pixels.size() / 2);
  for (qsizetype i = 0; i + 1 < pixels.size(); i += 2) {
    int x = pixels[i].toInt();
    int y = pixels[i + 1].toInt();
    *min_x = std::min(*min_x, x);
    *max_x = std::max(*max_x, x + 1);
    *min_y = std::min(*min_y, y);
    *max_y = std::max(*max_y, y + 1);
    points.emplace_back(Point{x, y});
  }
}

static void move_rects(std::vector<QRect>& rects, int move_x, int move_y) {
  for (auto& rect : rects) {
    rect.translate(move_x, move_y);
  }
}

static void move_points(std::vector<Point>& points, int move_x, int move_y) {
  for (auto& point : points) {
    point.x += move_x;
    point.y += move_y;
  }
}

void RobotMap::generate_map() {
  m_map = Map();
  m_map.size_x = m_map_json["size"].toObject()["x"].toInt();
  m_map.size_y = m_map_json["size"].toObject()["y"].toInt();
  m_map.pixel_size = m_map_json["pixelSize"].toInt();
  int min_x{std::numeric_limits<int>::max()},
      max_x{std::numeric_limits<int>::min()},
      min_y{std::numeric_limits<int>::max()},
      max_y{std::numeric_limits<int>::min()};

  // Because we merge layers together, pixels/blocks may occur multiple times in
  // a Layer. As layers are already a performance problem, we have to remove
  // duplicates. We use a map of Y-coordinates to a vector of X-spans.
  std::unordered_map<std::string,
                     std::map<int, std::vector<std::pair<int, int>>>>
      layer_spans;

  for (const auto&& layer : m_map_json["layers"].toArray()) {
    const auto& layer_obj = layer.toObject();
    auto layer_type = layer_obj["type"].toString().toStdString();
    if (layer_type == "segment") {
      layer_type = "floor";
    }
    if (layer_type == "floor" || layer_type == "wall") {
      if (m_map_version == 1) {
        const auto& pixels = layer_obj["pixels"].toArray();
        if (pixels.size() < 2 || (pixels.size() % 2) != 0) {
          qDebug().nospace()
              << "Invalid layer, has " << pixels.size() << " points";
          continue;
        }
        for (qsizetype i = 0; i + 1 < pixels.size(); i += 2) {
          const int x = pixels[i].toInt();
          const int y = pixels[i + 1].toInt();
          layer_spans[layer_type][y].emplace_back(x, x);
        }
      } else if (m_map_version == 2) {
        const auto& compressed_pixels = layer_obj["compressedPixels"].toArray();
        if (compressed_pixels.size() < 3 ||
            (compressed_pixels.size() % 3) != 0) {
          qDebug().nospace()
              << "Invalid layer, has " << compressed_pixels.size() << " points";
          continue;
        }
        for (qsizetype i = 0; i + 2 < compressed_pixels.size(); i += 3) {
          const int x = compressed_pixels[i].toInt();
          const int y = compressed_pixels[i + 1].toInt();
          const int count = compressed_pixels[i + 2].toInt();
          if (count > 0) {
            layer_spans[layer_type][y].emplace_back(x, x + count - 1);
          }
        }
      }
    }
  }

  // Merge spans and create QRects
  for (auto& [type, rows] : layer_spans) {
    auto& layer_rects = m_map.layers[type].rects;
    for (auto& [y, spans] : rows) {
      if (spans.empty()) continue;
      std::sort(spans.begin(), spans.end());

      int current_start = spans[0].first;
      int current_end = spans[0].second;

      for (size_t i = 1; i < spans.size(); ++i) {
        if (spans[i].first <= current_end + 1) {
          current_end = std::max(current_end, spans[i].second);
        } else {
          int block_x = current_start * m_map.pixel_size;
          int block_y = y * m_map.pixel_size;
          int width = (current_end - current_start + 1) * m_map.pixel_size;
          int height = m_map.pixel_size;

          min_x = std::min(min_x, block_x);
          max_x = std::max(max_x, block_x + width);
          min_y = std::min(min_y, block_y);
          max_y = std::max(max_y, block_y + height);

          layer_rects.emplace_back(block_x, block_y, width, height);

          current_start = spans[i].first;
          current_end = spans[i].second;
        }
      }
      int block_x = current_start * m_map.pixel_size;
      int block_y = y * m_map.pixel_size;
      int width = (current_end - current_start + 1) * m_map.pixel_size;
      int height = m_map.pixel_size;

      min_x = std::min(min_x, block_x);
      max_x = std::max(max_x, block_x + width);
      min_y = std::min(min_y, block_y);
      max_y = std::max(max_y, block_y + height);

      layer_rects.emplace_back(block_x, block_y, width, height);
    }
  }

  for (const auto&& entity : m_map_json["entities"].toArray()) {
    const auto& entity_obj = entity.toObject();
    const auto entity_class = entity_obj["__class"].toString();
    const auto entity_type = entity_obj["type"].toString();
    const auto entity_metadata = entity_obj["metaData"].toObject();
    const auto& points = entity_obj["points"].toArray();
    if (entity_class == "PointMapEntity" && points.size() != 2) {
      qDebug().nospace() << "Found PointMapEntity with not 2 points but "
                         << points.size() << ", ignoring";
      continue;
    }
    if (entity_type.endsWith("_position") && entity_class != "PointMapEntity") {
      qDebug() << entity_type << "entity must be a PointMapEntity, but was"
               << entity_class;
      continue;
    }
    const auto entity_type_str = entity_type.toStdString();
    Entity map_entity;
    map_entity.type = entity_type_str;
    map_entity.cls = entity_class.toStdString();
    parse_entity_points(points, map_entity.points, &min_x, &max_x, &min_y,
                        &max_y);
    map_entity.angle =
        (entity_metadata["angle"].toDouble() - 90.0) * M_PI / 180.0;

    if (map_entity.type == "robot_position") {
      // Insert robot position last to ensure the robot is drawn last
      m_map.entities.push_back(map_entity);
    } else {
      m_map.entities.insert(m_map.entities.begin(), map_entity);
    }
  }

  for (auto& layer : m_map.layers) {
    move_rects(layer.second.rects, -min_x, -min_y);
  }

  for (auto& entity : m_map.entities) {
    move_points(entity.points, -min_x, -min_y);
  }
  m_map.size_x = max_x - min_x;
  m_map.size_y = max_y - min_y;
  m_map.crop_x = min_x;
  m_map.crop_y = min_y;
  qDebug() << "Generated map with size" << m_map.size_x << "x" << m_map.size_y
           << "- cropped" << m_map.crop_x << "x" << m_map.crop_y;
}

bool RobotMap::is_valid() const { return m_valid; }

QString RobotMap::error_msg() const { return m_error; }

const Map& RobotMap::get_map() const { return m_map; }

const QJsonObject& RobotMap::get_map_json() const { return m_map_json; }

void RobotMap::reset() {
  m_valid = false;
  m_error = "No map data";
  emit signal_map_updated();
}

}  // namespace Valeronoi::state
