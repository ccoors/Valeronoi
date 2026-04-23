/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
 */
#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <catch.hpp>

#include "src/util/segment_generator.h"

TEST_CASE("SegmentGenerator thread safety and responsiveness", "[util]") {
  // We need a QCoreApplication for signals/slots to work if they are
  // cross-thread
  int argc = 1;
  char* argv[] = {(char*)"test"};
  QCoreApplication app(argc, argv);

  Valeronoi::util::SegmentGenerator generator;
  QSignalSpy spy(&generator,
                 &Valeronoi::util::SegmentGenerator::generated_segments);

  Valeronoi::state::RawMeasurements measurements;
  for (int i = 0; i < 100; ++i) {
    Valeronoi::state::Measurement m;
    m.x = i * 10;
    m.y = i * 10;
    m.average = i;
    m.wifi_id = 1;
    m.data = {static_cast<double>(i)};
    measurements.push_back(m);
  }

  SECTION("Frequent updates do not hang the thread") {
    for (int i = 0; i < 50; ++i) {
      generator.generate(measurements, Valeronoi::state::DISPLAY_MODE::Voronoi,
                         1);
    }

    // Give it some time to process at least the last one
    int attempts = 0;
    while (spy.count() == 0 && attempts < 100) {
      QCoreApplication::processEvents();
      QThread::msleep(100);
      attempts++;
    }

    CHECK(spy.count() > 0);
  }

  SECTION("Abort works correctly") {
    generator.generate(measurements, Valeronoi::state::DISPLAY_MODE::Voronoi,
                       1);
    // Destructor of generator (which calls m_abort = true and wait()) will
    // happen when it goes out of scope.
  }
}

TEST_CASE("SegmentGenerator simplification logic", "[util]") {
  // We need a QCoreApplication
  if (!QCoreApplication::instance()) {
    int argc = 1;
    char* argv[] = {(char*)"test"};
    new QCoreApplication(argc, argv);
  }

  Valeronoi::util::SegmentGenerator generator;
  QSignalSpy spy(&generator,
                 &Valeronoi::util::SegmentGenerator::generated_segments);

  Valeronoi::state::RawMeasurements measurements;
  // Two measurements that should be simplified to one
  measurements.push_back({10, 10, 1, {-50.0}, -50.0});
  measurements.push_back({11, 11, 1, {-60.0}, -60.0});

  // Simplify with factor 10
  generator.generate(measurements, Valeronoi::state::DISPLAY_MODE::DataPoints,
                     10);

  int attempts = 0;
  while (spy.count() == 0 && attempts < 50) {
    QCoreApplication::processEvents();
    QThread::msleep(100);
    attempts++;
  }

  REQUIRE(spy.count() > 0);
  auto segments = spy.takeFirst().at(0).value<Valeronoi::state::DataSegments>();
  CHECK(segments.size() == 1);
  CHECK(segments[0].x == 10);
  CHECK(segments[0].y == 10);
  CHECK(segments[0].value == Approx(-55.0));
}
