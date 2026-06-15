#include "Config/ScenarioData.hpp"

#include <functional>

#include "Phase/AnimationViewerPhase.hpp"
#include "Phase/PlayerTestPhase.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/TestMenuPhase.hpp"
#include "Phase/WaitPhase.hpp"

namespace {

/// @brief 型 T のパラメータを保持し、make() で T を生成する IPhaseMaker
/// @tparam T IPhase を継承するフェーズクラス（T::Param を持つこと）
template <typename T>
class PhaseMaker : public IPhaseMaker {
 public:
  explicit PhaseMaker(typename T::Param param) : m_param(std::move(param)) {}

  [[nodiscard]] std::unique_ptr<IPhase> make() const override {
    return std::make_unique<T>(m_param);
  }

 private:
  typename T::Param m_param;
};

/// @brief TOML 値から PhaseMaker<T> を生成する関数の型
using PhaseLoader = std::function<IPhaseMaker::Ptr(const s3d::TOMLValue&)>;

/// @brief 型 T に対する PhaseLoader を生成するヘルパー
/// @tparam T IPhase を継承するフェーズクラス（T::Param を持つこと）
/// @param parse TOML 値から T::Param を生成するラムダ
template <typename T, typename F>
PhaseLoader MakeLoader(F&& parse) {
  return [p = std::forward<F>(parse)](const s3d::TOMLValue& step) {
    return std::make_shared<const PhaseMaker<T>>(p(step));
  };
}

/// @brief フェーズ名 → PhaseLoader のマップ
const s3d::HashTable<s3d::String, PhaseLoader> kPhaseLoaders{
    {U"player_test", MakeLoader<PlayerTestPhase>([](const s3d::TOMLValue&) {
       return PlayerTestPhase::Param{};
     })},
    {U"test_menu", MakeLoader<TestMenuPhase>([](const s3d::TOMLValue&) {
       return TestMenuPhase::Param{};
     })},
    {U"animation_viewer",
     MakeLoader<AnimationViewerPhase>([](const s3d::TOMLValue& step) {
       return AnimationViewerPhase::Param{
           .dataKey = step[U"param"].get<s3d::String>(),
       };
     })},
    {U"scenario", MakeLoader<ScenarioPhase>([](const s3d::TOMLValue& step) {
       return ScenarioPhase::Param{
           .sectionName = step[U"param"].get<s3d::String>(),
       };
     })},
    {U"wait", MakeLoader<WaitPhase>([](const s3d::TOMLValue& step) {
       return WaitPhase::Param{
           .duration = step[U"duration"].get<double>(),
       };
     })},
};

/// @brief TOML の 1 ステップ値を ScenarioStep に変換する
/// @note "make" アクションは別 Issue で対応予定のためエラーとする
ScenarioStep ParseStep(const s3d::TOMLValue& step) {
  const auto action = step[U"action"].get<s3d::String>();

  if (action == U"push" || action == U"reset") {
    const auto phaseName = step[U"phase"].get<s3d::String>();
    const auto it = kPhaseLoaders.find(phaseName);
    if (it == kPhaseLoaders.end()) {
      throw s3d::Error{U"ScenarioData::ParseStep: 未登録のフェーズ名 \"" +
                       phaseName + U"\""};
    }
    auto maker = it->second(step);
    if (action == U"push") {
      return StepPush{.maker = std::move(maker)};
    }
    return StepReset{.maker = std::move(maker)};
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

ScenarioData ScenarioData::FromToml(const s3d::TOMLValue& toml) {
  ScenarioData data;

  for (const auto& member : toml.tableView()) {
    s3d::Array<ScenarioStep> steps;
    for (const auto& step : member.value.tableArrayView()) {
      steps.push_back(ParseStep(step));
    }
    data.sections[member.name] = std::move(steps);
  }

  return data;
}
