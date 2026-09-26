#include "ArenaConfig.hpp"

#include "Config/TomlFields.hpp"

std::expected<ArenaConfig, String> ArenaConfig::FromToml(
    const TOMLValue& toml
) {
  TomlFields f{toml, U"ArenaConfig::FromToml"};
  auto cfg = f.wrap(
      ArenaConfig{
          .halfW = f.get<double>(U"half_w"),
          .halfD = f.get<double>(U"half_d"),
      }
  );
  if (!cfg) {
    return std::unexpected{std::move(cfg).error()};
  }

  // BoundarySystem の Clamp 下限（-half + radius）が上限（half - radius）を
  // 超えないよう、half は正でなければならない
  if (cfg->halfW <= 0.0 || cfg->halfD <= 0.0) {
    return std::unexpected{
        U"ArenaConfig::FromToml: half_w と half_d は正でなければなりません"
    };
  }

  return cfg;
}
