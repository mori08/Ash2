#include "Config/ScenarioData.hpp"

namespace {

/// @brief TOML の 1 ステップ値を ScenarioStep に変換する
/// @note "make" アクションは別 Issue で対応予定のためエラーとする
[[nodiscard]] std::expected<ScenarioStep, String> ParseStep(
    const TOMLValue& step, const PhaseLoaderTable& loaders) {
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
      return std::unexpected{U"ScenarioData::ParseStep: 未登録のフェーズ名 \"" +
                             *phaseName + U"\""};
    }
    auto maker = it->second(step);
    if (!maker) {
      return std::unexpected{std::move(maker).error()};
    }
    if (*action == U"push") {
      return StepPush{.maker = *std::move(maker)};
    }
    return StepReset{.maker = *std::move(maker)};
  }

  if (*action == U"make") {
    return std::unexpected{
        U"ScenarioData::ParseStep: \"make\" アクションは未対応です（Issue "
        U"#67 スコープ外）"};
  }

  return std::unexpected{U"ScenarioData::ParseStep: 不明なアクション \"" +
                         *action + U"\""};
}

}  // namespace

ScenarioData ScenarioData::FromToml(const TOMLValue& toml,
                                    const PhaseLoaderTable& loaders) {
  ScenarioData data;

  for (const auto& member : toml.tableView()) {
    Array<ScenarioStep> steps;
    for (const auto& step : member.value.tableArrayView()) {
      auto parsed = ParseStep(step, loaders);
      if (!parsed) {
        // Why not: ParseStep からの失敗は expected で伝播してくるが、
        // ここは設定読み込みの最上位境界であり、呼び出し元の
        // GameSetup は expected を扱わない設計のため throw
        // で致命として確定させる。
        throw Error{std::move(parsed).error()};
      }
      steps.push_back(*std::move(parsed));
    }
    data.sections[member.name] = std::move(steps);
  }

  return data;
}
