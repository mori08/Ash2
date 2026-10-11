#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/InputConfig.hpp"

namespace {

/// @brief 文字列を TOML として読み InputConfig へ変換する
std::expected<InputConfig, String> Parse(std::string_view toml) {
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
  REQUIRE(cfg.has_value());
  REQUIRE(cfg->deviceSwitchCursorDistance == 8.0);
  REQUIRE(cfg->navStickTilt == 0.5);
  REQUIRE(cfg->navStickRelease == 0.24);
}

TEST_CASE("InputConfig::FromToml - missing key returns unexpected") {
  REQUIRE_FALSE(
      Parse(
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.24\n"
      )
          .has_value()
  );
}

TEST_CASE(
    "InputConfig::FromToml - negative switch distance returns unexpected"
) {
  REQUIRE_FALSE(
      Parse(
          "device_switch_cursor_distance = -1.0\n"
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.24\n"
      )
          .has_value()
  );
}

TEST_CASE("InputConfig::FromToml - release not below tilt returns unexpected") {
  REQUIRE_FALSE(
      Parse(
          "device_switch_cursor_distance = 8.0\n"
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.5\n"
      )
          .has_value()
  );
}

TEST_CASE("InputConfig::FromToml - non-positive release returns unexpected") {
  REQUIRE_FALSE(
      Parse(
          "device_switch_cursor_distance = 8.0\n"
          "nav_stick_tilt = 0.5\n"
          "nav_stick_release = 0.0\n"
      )
          .has_value()
  );
}

TEST_CASE("InputConfig::FromToml - tilt above 1 returns unexpected") {
  REQUIRE_FALSE(
      Parse(
          "device_switch_cursor_distance = 8.0\n"
          "nav_stick_tilt = 1.5\n"
          "nav_stick_release = 0.24\n"
      )
          .has_value()
  );
}

#endif
