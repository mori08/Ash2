#include "Config/ScenarioData.hpp"

#include "Config/TomlTable.hpp"

namespace {

/// @brief ParseStep の結果：ステップと、それが参照するセクション名
struct ParsedStep {
  ScenarioStep step;
  /// 別のシナリオセクションを参照する場合のみ設定される
  Optional<String> referencedSection;
};

/// @brief TOML の 1 ステップ値を ParsedStep に変換する
/// @note "make" アクションは別 Issue で対応予定のためエラーとする
[[nodiscard]] ParsedStep ParseStep(
    const TOMLValue& step, const PhaseLoaderTable& loaders
) {
  const TomlTable t{step};
  const auto action = t.get<String>(U"action");

  if (action == U"push" || action == U"reset") {
    const auto it = loaders.find(t.get<String>(U"phase"));
    if (it == loaders.end()) {
      throw Error{U"シナリオの phase が未登録の名前"};
    }
    auto loaded = it->second(step);
    ScenarioStep parsedStep;
    if (action == U"push") {
      parsedStep = StepPush{.maker = std::move(loaded.maker)};
    } else {
      parsedStep = StepReset{.maker = std::move(loaded.maker)};
    }
    return ParsedStep{
        .step = std::move(parsedStep),
        .referencedSection = std::move(loaded.referencedSection),
    };
  }

  if (action == U"make") {
    throw Error{U"シナリオの make アクションは未対応"};
  }

  throw Error{U"シナリオの action が不明な値"};
}

/// @brief セクション間の参照関係：from セクションが to セクションを参照する
struct SectionRef {
  String from;
  String to;
};

}  // namespace

ScenarioData ScenarioData::FromToml(
    const TOMLValue& toml, const PhaseLoaderTable& loaders
) {
  ScenarioData data;
  Array<SectionRef> refs;

  for (const auto& member : toml.tableView()) {
    Array<ScenarioStep> steps;
    for (const auto& step : member.value.tableArrayView()) {
      auto parsed = ParseStep(step, loaders);
      if (parsed.referencedSection) {
        refs.push_back(
            {.from = member.name, .to = *std::move(parsed.referencedSection)}
        );
      }
      steps.push_back(std::move(parsed.step));
    }
    data.sections[member.name] = std::move(steps);
  }

  // 前方参照を許すため、全セクションを登録し終えてから検証する
  for (const auto& ref : refs) {
    if (!data.sections.contains(ref.to)) {
      throw Error{U"シナリオの参照先セクションがない"};
    }
  }

  return data;
}
