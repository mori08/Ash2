#include "Config/ScenarioData.hpp"

namespace {

/// @brief ParseStep の結果：ステップと、それが参照するセクション名
struct ParsedStep {
  ScenarioStep step;
  /// 別のシナリオセクションを参照する場合のみ設定される
  Optional<String> referencedSection;
};

/// @brief TOML の 1 ステップ値を ParsedStep に変換する
/// @note "make" アクションは別 Issue で対応予定のためエラーとする
[[nodiscard]] std::expected<ParsedStep, String> ParseStep(
    const TOMLValue& step, const PhaseLoaderTable& loaders
) {
  const auto action = step[U"action"].getOpt<String>();
  if (!action) {
    return std::unexpected{U"ScenarioData::ParseStep: action がありません"};
  }

  if (*action == U"push" || *action == U"reset") {
    const auto phaseName = step[U"phase"].getOpt<String>();
    if (!phaseName) {
      return std::unexpected{U"ScenarioData::ParseStep: phase がありません"};
    }
    const auto it = loaders.find(*phaseName);
    if (it == loaders.end()) {
      return std::unexpected{
          U"ScenarioData::ParseStep: 未登録のフェーズ名 \"" + *phaseName + U"\""
      };
    }
    auto loaded = it->second(step);
    if (!loaded) {
      return std::unexpected{std::move(loaded).error()};
    }
    ScenarioStep parsedStep;
    if (*action == U"push") {
      parsedStep = StepPush{.maker = std::move(loaded->maker)};
    } else {
      parsedStep = StepReset{.maker = std::move(loaded->maker)};
    }
    return ParsedStep{
        .step = std::move(parsedStep),
        .referencedSection = std::move(loaded->referencedSection),
    };
  }

  if (*action == U"make") {
    return std::unexpected{
        U"ScenarioData::ParseStep: \"make\" アクションは未対応です（Issue "
        U"#67 スコープ外）"
    };
  }

  return std::unexpected{
      U"ScenarioData::ParseStep: 不明なアクション \"" + *action + U"\""
  };
}

/// @brief セクション間の参照関係：from セクションが to セクションを参照する
struct SectionRef {
  String from;
  String to;
};

}  // namespace

std::expected<ScenarioData, String> ScenarioData::FromToml(
    const TOMLValue& toml, const PhaseLoaderTable& loaders
) {
  ScenarioData data;
  Array<SectionRef> refs;

  for (const auto& member : toml.tableView()) {
    Array<ScenarioStep> steps;
    for (const auto& step : member.value.tableArrayView()) {
      auto parsed = ParseStep(step, loaders);
      if (!parsed) {
        return std::unexpected{std::move(parsed).error()};
      }
      if (parsed->referencedSection) {
        refs.push_back(
            {.from = member.name, .to = *std::move(parsed->referencedSection)}
        );
      }
      steps.push_back(std::move(parsed->step));
    }
    data.sections[member.name] = std::move(steps);
  }

  // 前方参照を許すため、全セクションを登録し終えてから検証する
  for (const auto& ref : refs) {
    if (!data.sections.contains(ref.to)) {
      return std::unexpected{
          U"ScenarioData::FromToml: セクション \"{}\" が参照する \"{}\" があ"
          U"りません"_fmt(ref.from, ref.to)
      };
    }
  }

  return data;
}
