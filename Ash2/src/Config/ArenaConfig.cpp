#include "ArenaConfig.hpp"

#include "Config/TomlTable.hpp"

ArenaConfig ArenaConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  const ArenaConfig cfg{
      .halfW = t.get<double>(U"half_w"),
      .halfD = t.get<double>(U"half_d"),
  };

  // half は正でなければならない。BoundarySystem の Clamp 下限が上限を
  // 超えない条件（half >= radius）は、半径を知らないここでは確かめない
  if (cfg.halfW <= 0.0 || cfg.halfD <= 0.0) {
    throw Error{U"half_w か half_d が正でない"};
  }

  return cfg;
}
