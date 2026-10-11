#include "InputConfig.hpp"

#include "Config/TomlFields.hpp"

std::expected<InputConfig, String> InputConfig::FromToml(
    const TOMLValue& toml
) {
  TomlFields f{toml, U"InputConfig::FromToml"};
  auto cfg = f.wrap(
      InputConfig{
          .deviceSwitchCursorDistance =
              f.get<double>(U"device_switch_cursor_distance"),
          .navStickTilt = f.get<double>(U"nav_stick_tilt"),
          .navStickRelease = f.get<double>(U"nav_stick_release"),
      }
  );
  if (!cfg) {
    return std::unexpected{std::move(cfg).error()};
  }

  if (cfg->deviceSwitchCursorDistance < 0.0) {
    return std::unexpected{
        U"InputConfig::FromToml: device_switch_cursor_distance は 0 "
        U"以上でなければなりません"
    };
  }
  if (cfg->navStickRelease <= 0.0 ||
      cfg->navStickRelease >= cfg->navStickTilt || cfg->navStickTilt > 1.0) {
    return std::unexpected{
        U"InputConfig::FromToml: nav_stick_release と nav_stick_tilt は "
        U"0 < release < tilt <= 1 でなければなりません"
    };
  }

  return cfg;
}
