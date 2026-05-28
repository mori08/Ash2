#include <Siv3D.hpp>

#include "Phase/DemoPhase.hpp"
#include "Phase/PhaseParam.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/WaitPhase.hpp"

namespace {

/// @brief 型 T に対する PhaseEntry を生成するヘルパー
/// @tparam T IPhase を継承するフェーズクラス（T::Param を持つこと）
/// @param parse TOML 値から T::Param を生成するラムダ
/// @return PhaseEntry（parseParam + createPhase を持つ）
template <typename T, typename F>
PhaseEntry MakeEntry(F&& parse) {
  return PhaseEntry{
      .parseParam = [p = std::forward<F>(parse)](
                        const s3d::TOMLValue& v) -> PhaseParam { return p(v); },
      .createPhase = [](const PhaseParam& param) -> std::unique_ptr<IPhase> {
        const auto* p = std::get_if<typename T::Param>(&param);
        if (!p) {
          throw s3d::Error{
              U"PhaseEntry::createPhase: param の型が一致しません"};
        }
        return std::make_unique<T>(*p);
      },
  };
}

}  // namespace

PhaseRegistry MakeDefaultPhaseRegistry() {
  return {
      {U"demo", MakeEntry<DemoPhase>(
                    [](const s3d::TOMLValue&) { return DemoPhase::Param{}; })},
      {U"scenario", MakeEntry<ScenarioPhase>([](const s3d::TOMLValue& step) {
         return ScenarioPhase::Param{
             .sectionName = step[U"param"].get<s3d::String>(),
         };
       })},
      {U"wait", MakeEntry<WaitPhase>([](const s3d::TOMLValue& step) {
         return WaitPhase::Param{
             .duration = step[U"duration"].get<double>(),
         };
       })},
  };
}
