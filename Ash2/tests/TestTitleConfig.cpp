#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>

#include "Config/TitleConfig.hpp"

namespace {

/// 泡以外のトップレベルのキー
constexpr std::string_view kTopLevelToml =
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
    "surface_line_width = 1.5\n"
    "plant_color = \"#303030\"\n"
    "plant_stem_width = 3.0\n"
    "plant_spread_amplitude = 0.45\n";

/// @brief [bubble] の各値の TOML 表記（既定は正常値。差分だけ上書きする）
struct BubbleToml {
  std::string interval = "1.75";
  std::string jitter = "0.5";
  std::string rise = "120.0";
  std::string speed = "25.0";
  std::string radius = "5.0";
};

/// @brief [bubble] テーブルの TOML を組み立てる
std::string MakeBubbleToml(const BubbleToml& v = {}) {
  return "[bubble]\n"
         "color = \"#404040\"\n"
         "interval = " +
         v.interval +
         "\n"
         "interval_jitter = " +
         v.jitter +
         "\n"
         "pop_margin = 5.0\n"
         "far_scale = 0.35\n"
         "pop_alpha = 0.8\n"
         "rise = " +
         v.rise + "\nspeed = " + v.speed + "\nradius = " + v.radius + "\n";
}

/// @brief トップレベルのキーに続けて [bubble] を置いた TOML を読む
TitleConfig ParseWithBubble(const BubbleToml& bubble = {}) {
  const std::string toml = std::string{kTopLevelToml} + MakeBubbleToml(bubble);
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  return TitleConfig::FromToml(reader);
}

}  // namespace

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
      "surface_line_width = 1.5\n"
      "plant_color = \"#303030\"\n"
      "plant_stem_width = 3.0\n"
      "plant_spread_amplitude = 0.45\n"
      "[[plant_stems]]\n"
      "dx = -9.0\n"
      "height = 36.0\n"
      "[[plant_stems]]\n"
      "dx = 9.0\n"
      "height = 28.0\n"
      "[[plants]]\n"
      "x = 90.0\n"
      "period = 8.0\n"
      "offset = 0.25\n"
      "[bubble]\n"
      "color = \"#404040\"\n"
      "interval = 1.75\n"
      "interval_jitter = 0.5\n"
      "pop_margin = 5.0\n"
      "far_scale = 0.35\n"
      "pop_alpha = 0.8\n"
      "rise = 120.0\n"
      "speed = 25.0\n"
      "radius = 5.0\n";
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
  REQUIRE(cfg.plantColor == ColorF{Color{0x30, 0x30, 0x30}});
  REQUIRE(cfg.plantStemWidth == 3.0);
  REQUIRE(cfg.plantSpreadAmplitude == 0.45);
  REQUIRE(cfg.plantStems.size() == 2);
  REQUIRE(cfg.plantStems[0].dx == -9.0);
  REQUIRE(cfg.plantStems[0].height == 36.0);
  REQUIRE(cfg.plantStems[1].dx == 9.0);
  REQUIRE(cfg.plantStems[1].height == 28.0);
  REQUIRE(cfg.plants.size() == 1);
  REQUIRE(cfg.plants[0].x == 90.0);
  REQUIRE(cfg.plants[0].period == 8.0);
  REQUIRE(cfg.plants[0].offset == 0.25);
  REQUIRE(cfg.bubble.color == ColorF{Color{0x40, 0x40, 0x40}});
  REQUIRE(cfg.bubble.interval == 1.75);
  REQUIRE(cfg.bubble.intervalJitter == 0.5);
  REQUIRE(cfg.bubble.popMargin == 5.0);
  REQUIRE(cfg.bubble.farScale == 0.35);
  REQUIRE(cfg.bubble.popAlpha == 0.8);
  REQUIRE(cfg.bubble.rise == 120.0);
  REQUIRE(cfg.bubble.speed == 25.0);
  REQUIRE(cfg.bubble.radius == 5.0);
}

TEST_CASE("TitleConfig::FromToml - plant arrays may be omitted") {
  const auto cfg = ParseWithBubble();
  REQUIRE(cfg.plantStems.isEmpty());
  REQUIRE(cfg.plants.isEmpty());
}

TEST_CASE("TitleConfig::FromToml - plant period of zero throws") {
  const std::string toml =
      std::string{kTopLevelToml} +
      "[[plants]]\n"
      "x = 90.0\n"
      "period = 0.0\n"
      "offset = 0.25\n" +
      MakeBubbleToml();
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  REQUIRE_THROWS_AS((void)TitleConfig::FromToml(reader), Error);
}

TEST_CASE("TitleConfig::FromToml - missing bubble table throws") {
  const TOMLReader reader{
      MemoryViewReader{kTopLevelToml.data(), kTopLevelToml.size()}
  };
  REQUIRE_THROWS_AS((void)TitleConfig::FromToml(reader), Error);
}

TEST_CASE("TitleConfig::FromToml - missing bubble key throws") {
  const std::string toml =
      std::string{kTopLevelToml} +
      "[bubble]\n"
      "color = \"#404040\"\n";
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  REQUIRE_THROWS_AS((void)TitleConfig::FromToml(reader), Error);
}

TEST_CASE("TitleConfig::FromToml - non-positive bubble value throws") {
  REQUIRE_THROWS_AS((void)ParseWithBubble({.interval = "0.0"}), Error);
  REQUIRE_THROWS_AS((void)ParseWithBubble({.interval = "-1.0"}), Error);
  REQUIRE_THROWS_AS((void)ParseWithBubble({.rise = "0.0"}), Error);
  REQUIRE_THROWS_AS((void)ParseWithBubble({.speed = "0.0"}), Error);
  REQUIRE_THROWS_AS((void)ParseWithBubble({.radius = "0.0"}), Error);
}

TEST_CASE("TitleConfig::FromToml - bubble jitter out of range throws") {
  REQUIRE_THROWS_AS((void)ParseWithBubble({.jitter = "1.0"}), Error);
  REQUIRE_THROWS_AS((void)ParseWithBubble({.jitter = "-0.1"}), Error);
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
