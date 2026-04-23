/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
 */
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <catch.hpp>

#include "src/state/wifi_collection.h"

using namespace Valeronoi::state;
using namespace Valeronoi::robot;

TEST_CASE("wifi_collection basic functionality", "[state]") {
  wifi_collection collection;

  SECTION("Add wifi and get ID") {
    WifiInformation wifi1(-50.0, "SSID1", "BSSID1");
    WifiInformation wifi2(-60.0, "SSID2", "BSSID2");

    int id1 = collection.add_wifi(wifi1);
    int id2 = collection.add_wifi(wifi2);

    CHECK(id1 == 0);
    CHECK(id2 == 1);
    CHECK(collection.get_wifi_id("BSSID1") == 0);
    CHECK(collection.get_wifi_id("BSSID2") == 1);
    CHECK(collection.get_wifi_id("UNKNOWN") == -1);
  }

  SECTION("Get or create wifi ID") {
    WifiInformation wifi1(-50.0, "SSID1", "BSSID1");

    int id1 = collection.get_or_create_wifi_id(wifi1);
    CHECK(id1 == 0);

    // Should return same ID for same BSSID
    int id1_again = collection.get_or_create_wifi_id(wifi1);
    CHECK(id1_again == 0);
    CHECK(collection.get_known_wifis().size() == 1);
  }

  SECTION("JSON serialization") {
    collection.add_wifi(WifiInformation(-50.0, "SSID1", "BSSID1"));
    QJsonArray json = collection.get_json();
    CHECK(json.size() == 1);
    CHECK(json[0].toObject()["ssid"].toString() == "SSID1");

    wifi_collection collection2;
    collection2.set_json(json);
    CHECK(collection2.get_known_wifis().size() == 1);
    CHECK(collection2.get_wifi_id("BSSID1") == 0);
  }

  SECTION("Signals") {
    QSignalSpy spyUpdated(&collection,
                          &wifi_collection::signal_wifi_list_updated);
    QSignalSpy spyNew(&collection, &wifi_collection::signal_new_wifi_added);

    collection.add_wifi(WifiInformation(-50.0, "SSID1", "BSSID1"));

    CHECK(spyUpdated.count() == 1);
    CHECK(spyNew.count() == 1);
  }

  SECTION("Clear") {
    collection.add_wifi(WifiInformation(-50.0, "SSID1", "BSSID1"));
    collection.clear();
    CHECK(collection.get_known_wifis().isEmpty());
  }
}
