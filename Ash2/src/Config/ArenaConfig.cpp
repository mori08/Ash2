#include "ArenaConfig.hpp"

#include "Config/TomlTable.hpp"

ArenaConfig ArenaConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  const ArenaConfig cfg{
      .halfW = t.get<double>(U"half_w"),
      .halfD = t.get<double>(U"half_d"),
  };

  // BoundarySystem の Clamp 下限（-half + radius）が上限（half - radius）を
  // 超えないよう、half は正でなければならない
  if (cfg.halfW <= 0.0 || cfg.halfD <= 0.0) {
    throw Error{U"half_w か half_d が正でない"};
  }

  return cfg;
}
