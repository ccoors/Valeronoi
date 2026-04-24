/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
 */
#include <catch.hpp>

#include "src/robot/wifi_information.h"

using namespace Valeronoi::robot;

TEST_CASE("WifiInformation basic functionality", "[robot]") {
  SECTION("Default constructor") {
    WifiInformation wifi;
    CHECK(wifi.ssid() == "Unknown WiFi");
    CHECK(wifi.bssid() == "00:00:00:00:00:00");
    CHECK(wifi.signal() == 0.0);
    CHECK_FALSE(wifi.has_valid_signal());
  }

  SECTION("Constructor with parameters") {
    WifiInformation wifi(-50.0, "MySSID", "11:22:33:44:55:66");
    CHECK(wifi.ssid() == "MySSID");
    CHECK(wifi.bssid() == "11:22:33:44:55:66");
    CHECK(wifi.signal() == -50.0);
    CHECK(wifi.has_valid_signal());
  }

  SECTION("JSON serialization/deserialization") {
    QJsonObject obj;
    obj.insert("ssid", "TestSSID");
    obj.insert("bssid", "AA:BB:CC:DD:EE:FF");
    obj.insert("signal", -65.5);

    WifiInformation wifi(obj);
    CHECK(wifi.ssid() == "TestSSID");
    CHECK(wifi.bssid() == "AA:BB:CC:DD:EE:FF");
    CHECK(wifi.signal() == -65.5);

    QJsonObject serialized = wifi.get_json();
    CHECK(serialized["ssid"].toString() == "TestSSID");
    CHECK(serialized["bssid"].toString() == "AA:BB:CC:DD:EE:FF");
    CHECK(serialized["signal"].toDouble() == -65.5);
  }

  SECTION("Setters") {
    WifiInformation wifi;
    wifi.set_ssid("NewSSID");
    wifi.set_bssid("01:02:03:04:05:06");
    wifi.set_signal(-70.0);

    CHECK(wifi.ssid() == "NewSSID");
    CHECK(wifi.bssid() == "01:02:03:04:05:06");
    CHECK(wifi.signal() == -70.0);
    CHECK(wifi.has_valid_signal());
  }
}
