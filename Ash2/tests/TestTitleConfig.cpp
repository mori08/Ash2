#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/TitleConfig.hpp"

TEST_CASE("TitleConfig::FromToml - parses all fields correctly") {
  constexpr std::string_view kToml =
      "logo_y = 160.0\n"
      "menu_y = 440.0\n"
      "menu_spacing = 60.0\n"
      "button_width = 200.0\n"
      "button_height = 48.0\n"
      "background_color = \"#E0E0E0\"\n"
      "surface_line_color = \"#0F0F0F\"\n"
      "logo_color = \"#101010\"\n"
      "button_normal_color = \"#8C8C8C\"\n"
      "button_focused_color = \"#202020\"\n"
      "surface_y = 360.0\n"
      "surface_line_width = 1.5\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  const auto cfg = TitleConfig::FromToml(reader);
  REQUIRE(cfg.logoY == 160.0);
  REQUIRE(cfg.menuY == 440.0);
  REQUIRE(cfg.menuSpacing == 60.0);
  REQUIRE(cfg.buttonWidth == 200.0);
  REQUIRE(cfg.buttonHeight == 48.0);
  REQUIRE(cfg.backgroundColor == ColorF{Color{0xE0, 0xE0, 0xE0}});
  REQUIRE(cfg.surfaceLineColor == ColorF{Color{0x0F, 0x0F, 0x0F}});
  REQUIRE(cfg.logoColor == ColorF{Color{0x10, 0x10, 0x10}});
  REQUIRE(cfg.buttonNormalColor == ColorF{Color{0x8C, 0x8C, 0x8C}});
  REQUIRE(cfg.buttonFocusedColor == ColorF{Color{0x20, 0x20, 0x20}});
  REQUIRE(cfg.surfaceY == 360.0);
  REQUIRE(cfg.surfaceLineWidth == 1.5);
}

TEST_CASE("TitleConfig::FromToml - missing logo_y throws") {
  constexpr std::string_view kToml =
      "menu_y = 440.0\n"
      "menu_spacing = 60.0\n"
      "button_width = 200.0\n"
      "button_height = 48.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS((void)TitleConfig::FromToml(reader), Error);
}

TEST_CASE("TitleConfig::FromToml - missing menu key throws") {
  constexpr std::string_view kToml = "logo_y = 160.0\n";
  const TOMLReader reader{MemoryViewReader{kToml.data(), kToml.size()}};
  REQUIRE_THROWS_AS((void)TitleConfig::FromToml(reader), Error);
}

#endif
