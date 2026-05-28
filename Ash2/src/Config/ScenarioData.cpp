#include "Config/ScenarioData.hpp"

#include "Phase/PhaseParam.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "Phase/ScenarioStep.hpp"

namespace {

/// @brief TOML の 1 ステップ値を ScenarioStep に変換する
/// @param step TOML ステップ値
/// @param registry フェーズ名 → PhaseEntry の対応表
/// @return ScenarioStep（StepPush または StepReset）
/// @note "make" アクションは別 Issue で対応予定のためエラーとする
ScenarioStep ParseStep(const s3d::TOMLValue& step,
                       const PhaseRegistry& registry) {
  const auto action = step[U"action"].get<s3d::String>();

  if (action == U"push") {
    const auto phaseName = step[U"phase"].get<s3d::String>();
    if (!registry.contains(phaseName)) {
      throw s3d::Error{U"ScenarioData::ParseStep: 未登録のフェーズ名 \"" +
                       phaseName + U"\""};
    }
    return StepPush{.phaseName = phaseName,
                    .param = registry.at(phaseName).parseParam(step)};
  }

  if (action == U"reset") {
    const auto phaseName = step[U"phase"].get<s3d::String>();
    if (!registry.contains(phaseName)) {
      throw s3d::Error{U"ScenarioData::ParseStep: 未登録のフェーズ名 \"" +
                       phaseName + U"\""};
    }
    return StepReset{.phaseName = phaseName,
                     .param = registry.at(phaseName).parseParam(step)};
  }

  if (action == U"make") {
    throw s3d::Error{
        U"ScenarioData::FromToml: \"make\" アクションは未対応です（Issue #67 "
        U"スコープ外）"};
  }

  throw s3d::Error{U"ScenarioData::FromToml: 不明なアクション \"" + action +
                   U"\""};
}

}  // namespace

ScenarioData ScenarioData::FromToml(const s3d::TOMLValue& toml,
                                    const PhaseRegistry& registry) {
  ScenarioData data;
  for (const auto& member : toml.tableView()) {
    s3d::Array<ScenarioStep> steps;
    for (const auto& step : member.value.tableArrayView()) {
      steps.push_back(ParseStep(step, registry));
    }
    data.sections[member.name] = std::move(steps);
  }
  return data;
}
