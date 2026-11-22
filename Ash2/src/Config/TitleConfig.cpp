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
      .backgroundColor = t.get<ColorF>(U"background_color"),
      .surfaceLineColor = t.get<ColorF>(U"surface_line_color"),
      .logoColor = t.get<ColorF>(U"logo_color"),
      .buttonNormalColor = t.get<ColorF>(U"button_normal_color"),
      .buttonFocusedColor = t.get<ColorF>(U"button_focused_color"),
      .surfaceY = t.get<double>(U"surface_y"),
      .surfaceLineWidth = t.get<double>(U"surface_line_width"),
  };
}
