#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/InputConfig.hpp"

namespace {

/// @brief 文字列を TOML として読み InputConfig へ変換する
InputConfig Parse(std::string_view toml) {
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  return InputConfig::FromToml(reader);
}

}  // namespace

TEST_CASE("InputConfig::FromToml - parses all fields correctly") {
  const auto cfg = Parse(
      "device_switch_cursor_distance = 8.0\n"
      "nav_stick_tilt = 0.5\n"
      "nav_stick_release = 0.24\n"
  );
  REQUIRE(cfg.deviceSwitchCursorDistance == 8.0);
  REQUIRE(cfg.navStickTilt == 0.5);
  REQUIRE(cfg.navStickRelease == 0.24);
}

TEST_CASE("InputConfig::FromToml - missing key throws") {
  REQUIRE_THROWS_AS(
      (void)Parse(
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.24\n"
      ),
      Error
  );
}

TEST_CASE("InputConfig::FromToml - negative switch distance throws") {
  REQUIRE_THROWS_AS(
      (void)Parse(
          "device_switch_cursor_distance = -1.0\n"
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.24\n"
      ),
      Error
  );
}

TEST_CASE("InputConfig::FromToml - release not below tilt throws") {
  REQUIRE_THROWS_AS(
      (void)Parse(
          "device_switch_cursor_distance = 8.0\n"
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.5\n"
      ),
      Error
  );
}

TEST_CASE("InputConfig::FromToml - non-positive release throws") {
  REQUIRE_THROWS_AS(
      (void)Parse(
          "device_switch_cursor_distance = 8.0\n"
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.0\n"
      ),
      Error
  );
}

TEST_CASE("InputConfig::FromToml - tilt above 1 throws") {
  REQUIRE_THROWS_AS(
      (void)Parse(
          "device_switch_cursor_distance = 8.0\n"
          "nav_stick_tilt = 1.5\n"
          "nav_stick_release = 0.24\n"
      ),
      Error
  );
}

#endif
