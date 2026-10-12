#include "TitleConfig.hpp"

#include "Config/TomlFields.hpp"

std::expected<TitleConfig, String> TitleConfig::FromToml(
    const TOMLValue& toml
) {
  TomlFields f{toml, U"TitleConfig::FromToml"};
  return f.wrap(TitleConfig{.logoY = f.get<double>(U"logo_y")});
}
