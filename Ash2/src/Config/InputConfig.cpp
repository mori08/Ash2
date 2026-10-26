#include "InputConfig.hpp"

#include "Config/TomlTable.hpp"

InputConfig InputConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  const InputConfig cfg{
      .deviceSwitchCursorDistance =
          t.get<double>(U"device_switch_cursor_distance"),
      .navStickTilt = t.get<double>(U"nav_stick_tilt"),
      .navStickRelease = t.get<double>(U"nav_stick_release"),
  };

  if (cfg.deviceSwitchCursorDistance < 0.0) {
    throw Error{U"device_switch_cursor_distance が負"};
  }
  if (cfg.navStickRelease <= 0.0 || cfg.navStickRelease >= cfg.navStickTilt ||
      cfg.navStickTilt > 1.0) {
    throw Error{U"nav_stick の閾値が 0 < release < tilt <= 1 でない"};
  }

  return cfg;
}
