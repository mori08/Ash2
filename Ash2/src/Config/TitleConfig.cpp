#include "TitleConfig.hpp"

#include "Config/TomlTable.hpp"

TitleConfig TitleConfig::FromToml(const TOMLValue& toml) {
  const TomlTable t{toml};
  return TitleConfig{
      .logoY = t.get<double>(U"logo_y"),
      .menuY = t.get<double>(U"menu_y"),
      .menuSpacing = t.get<double>(U"menu_spacing"),
      .buttonWidth = t.get<double>(U"button_width"),
      .buttonHeight = t.get<double>(U"button_height"),
  };
}
