#include "TitleConfig.hpp"

#include "Config/TomlTable.hpp"

namespace {

/// @brief TOML のテーブルの配列を読む。キーがなければ空を返す
/// @throw Error キーがあるのにテーブルの配列でないとき
template <class T, class ParseFn>
[[nodiscard]] Array<T>
ParseTableArray(const TOMLValue& toml, StringView key, ParseFn parse) {
  const auto value = TomlMember(toml, key);
  if (value.isEmpty()) return {};
  if (!value.isTableArray()) {
    throw Error{U"タイトル設定の植物がテーブルの配列でない"};
  }
  Array<T> result;
  for (const auto& elementToml : value.tableArrayView()) {
    result.push_back(parse(elementToml));
  }
  return result;
}

[[nodiscard]] PlantStemConfig ParsePlantStem(const TOMLValue& toml) {
  const TomlTable t{toml};
  return PlantStemConfig{
      .dx = t.get<double>(U"dx"),
      .height = t.get<double>(U"height"),
  };
}

[[nodiscard]] PlantConfig ParsePlant(const TOMLValue& toml) {
  const TomlTable t{toml};
  const PlantConfig plant{
      .x = t.get<double>(U"x"),
      .period = t.get<double>(U"period"),
      .offset = t.get<double>(U"offset"),
  };
  // 揺れの角度を period で割るため、0 以下は許さない
  if (plant.period <= 0.0) {
    throw Error{U"植物の period が 0 以下"};
  }
  return plant;
}

/// @brief 0 より大きい必須の数値を読む
/// @throw Error キーの欠落・型違い、0 以下のとき
[[nodiscard]] double ParsePositive(const TomlTable& table, StringView key) {
  const auto value = table.get<double>(key);
  if (value <= 0.0) {
    throw Error{U"泡の {} が 0 以下"_fmt(key)};
  }
  return value;
}

[[nodiscard]] BubbleConfig ParseBubble(const TOMLValue& toml) {
  const TomlTable t{TomlMember(toml, U"bubble")};
  const BubbleConfig bubble{
      .color = t.get<ColorF>(U"color"),
      .interval = ParsePositive(t, U"interval"),
      .intervalJitter = t.get<double>(U"interval_jitter"),
      .popMargin = t.get<double>(U"pop_margin"),
      .farScale = t.get<double>(U"far_scale"),
      .popAlpha = t.get<double>(U"pop_alpha"),
      .rise = ParsePositive(t, U"rise"),
      .speed = ParsePositive(t, U"speed"),
      .radius = ParsePositive(t, U"radius"),
  };
  // 間隔が 0 以下になりうると、泡を出すループが終わらない
  if (bubble.intervalJitter < 0.0 || bubble.intervalJitter >= 1.0) {
    throw Error{U"泡の interval_jitter が 0 以上 1 未満でない"};
  }
  return bubble;
}

}  // namespace

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
      .plantColor = t.get<ColorF>(U"plant_color"),
      .plantStemWidth = t.get<double>(U"plant_stem_width"),
      .plantSpreadAmplitude = t.get<double>(U"plant_spread_amplitude"),
      .plantStems = ParseTableArray<PlantStemConfig>(
          toml, U"plant_stems", ParsePlantStem
      ),
      .plants = ParseTableArray<PlantConfig>(toml, U"plants", ParsePlant),
      .bubble = ParseBubble(toml),
  };
}
