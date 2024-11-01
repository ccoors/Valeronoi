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
#include "robot_map.h"

#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Valeronoi::state {
RobotMap::RobotMap(QObject *parent) : QObject(parent) { reset(); }

void RobotMap::update_map_json(const QString &json) {
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

void RobotMap::update_map_json(const QJsonObject &json_object) {
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

template <typename T>
static void map_pixels(const QJsonArray &pixels, T &blocks, int pixel_size,
                       int *min_x, int *max_x, int *min_y, int *max_y) {
  blocks.reserve(pixels.size() / 2);
  for (qsizetype i = 0; i + 1 < pixels.size(); i += 2) {
    int block_x = pixels[i].toInt() * pixel_size;
    int block_y = pixels[i + 1].toInt() * pixel_size;
    *min_x = std::min(*min_x, block_x);
    *max_x = std::max(*max_x, block_x + pixel_size);
    *min_y = std::min(*min_y, block_y);
    *max_y = std::max(*max_y, block_y + pixel_size);
    blocks.push_back({block_x, block_y});
  }
}

template <typename T>
static void map_pixels(const std::unordered_set<Block> &pixels, T &blocks,
                       int pixel_size, int *min_x, int *max_x, int *min_y,
                       int *max_y) {
  blocks.reserve(pixels.size() / 2);
  for (const auto &block : pixels) {
    int block_x = block.x * pixel_size;
    int block_y = block.y * pixel_size;
    *min_x = std::min(*min_x, block_x);
    *max_x = std::max(*max_x, block_x + pixel_size);
    *min_y = std::min(*min_y, block_y);
    *max_y = std::max(*max_y, block_y + pixel_size);
    blocks.push_back({block_x, block_y});
  }
}

template <typename T>
static void move_pixels(T &blocks, int move_x, int move_y) {
  for (auto &block : blocks) {
    block.x += move_x;
    block.y += move_y;
  }
}

void RobotMap::generate_map() {
  m_map = Map();
  m_map.size_x = m_map_json["size"].toObject()["x"].toInt();
  m_map.size_y = m_map_json["size"].toObject()["y"].toInt();
  m_map.pixel_size = m_map_json["pixelSize"].toInt();
  int min_x{m_map.size_x}, max_x{0}, min_y{m_map.size_y}, max_y{0};

  // Because we merge layers together, pixels/blocks may occur multiple times in
  // a Layer. As layers are already a performance problem, we have to remove
  // duplicates by using an unordered_set.
  std::unordered_map<std::string, std::unordered_set<Block>> floor_pixels;
  for (const auto &&layer : m_map_json["layers"].toArray()) {
    const auto &layer_obj = layer.toObject();
    auto layer_type = layer_obj["type"].toString().toStdString();
    if (layer_type == "segment") {
      // Pretend segments are floor as well, because future Valetudo Versions
      // will not have "floor". Dreame Robots don't have a floor layer at all.
      layer_type = "floor";
    }
    if (layer_type == "floor" || layer_type == "wall") {
      if (m_map_version == 1) {
        const auto &pixels = layer_obj["pixels"].toArray();
        if ((pixels.size() % 2) != 0 || pixels.size() < 2) {
          qDebug().nospace()
              << "Invalid layer, has " << pixels.size() << " points";
          continue;
        }

        floor_pixels[layer_type].reserve(pixels.size() / 2);
        for (qsizetype i = 0; i + 1 < pixels.size(); i += 2) {
          const int block_x = pixels[i].toInt();
          const int block_y = pixels[i + 1].toInt();
          floor_pixels[layer_type].insert({block_x, block_y});
        }
      } else if (m_map_version == 2) {
        const auto &compressed_pixels = layer_obj["compressedPixels"].toArray();
        if ((compressed_pixels.size() % 3) != 0 ||
            compressed_pixels.size() < 3) {
          qDebug().nospace()
              << "Invalid layer, has " << compressed_pixels.size() << " points";
          continue;
        }

        for (qsizetype i = 0; i + 1 < compressed_pixels.size(); i += 3) {
          const int block_x = compressed_pixels[i].toInt();
          const int block_y = compressed_pixels[i + 1].toInt();
          const int block_count = compressed_pixels[i + 2].toInt();
          for (qsizetype j = 0; j < block_count; j++) {
            const auto pixel_x = static_cast<int>(block_x + j);
            floor_pixels[layer_type].insert({pixel_x, block_y});
          }
        }
      }
    }
  }
  for (const auto &layer : floor_pixels) {
    map_pixels(layer.second, m_map.layers[layer.first].blocks, m_map.pixel_size,
               &min_x, &max_x, &min_y, &max_y);
  }

  for (const auto &&entity : m_map_json["entities"].toArray()) {
    const auto &entity_obj = entity.toObject();
    const auto entity_class = entity_obj["__class"].toString();
    const auto entity_type = entity_obj["type"].toString();
    const auto entity_metadata = entity_obj["metaData"].toObject();
    const auto &points = entity_obj["points"].toArray();
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
    map_pixels(points, map_entity.points, 1, &min_x, &max_x, &min_y, &max_y);
    map_entity.angle =
        (entity_metadata["angle"].toDouble() - 90.0) * M_PI / 180.0;

    if (map_entity.type == "robot_position") {
      // Insert robot position last to ensure the robot is drawn last
      m_map.entities.push_back(map_entity);
    } else {
      m_map.entities.insert(m_map.entities.begin(), map_entity);
    }
  }

  for (auto &layer : m_map.layers) {
    move_pixels(layer.second.blocks, -min_x, -min_y);
  }

  for (auto &layer : m_map.entities) {
    move_pixels(layer.points, -min_x, -min_y);
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

const Map &RobotMap::get_map() const { return m_map; }

const QJsonObject &RobotMap::get_map_json() const { return m_map_json; }

void RobotMap::reset() {
  m_valid = false;
  m_error = "No map data";
  emit signal_map_updated();
}

}  // namespace Valeronoi::state
