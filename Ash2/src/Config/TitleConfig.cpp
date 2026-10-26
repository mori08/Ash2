#include "TitleConfig.hpp"

#include "Config/TomlTable.hpp"

TitleConfig TitleConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  return TitleConfig{.logoY = t.get<double>(U"logo_y")};
}
