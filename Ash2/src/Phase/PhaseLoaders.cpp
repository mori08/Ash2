#include "Phase/PhaseLoaders.hpp"

#include "Config/TomlTable.hpp"
#include "Phase/AnimationViewerPhase.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/StagePhase.hpp"
#include "Phase/TestMenuPhase.hpp"
#include "Phase/TitlePhase.hpp"
#include "Phase/WaitPhase.hpp"

namespace {

/// @brief IPhase を継承し、const Param& から構築可能な Param を持つフェーズ型
template <typename T>
concept PhaseWithParam =
    std::derived_from<T, IPhase> &&
    std::move_constructible<typename T::Param> &&
    std::constructible_from<T, const typename T::Param&>;

/// @brief 別のシナリオセクションを参照する Param
/// @note ScenarioPhase::Param の sectionName がこれに該当する
template <typename P>
concept SectionRefParam = requires(const P& p) {
  { p.sectionName } -> std::convertible_to<String>;
};

/// @brief 型 T のパラメータを保持し、make() で T を生成する IPhaseMaker
template <PhaseWithParam T>
class PhaseMaker : public IPhaseMaker {
 public:
  explicit PhaseMaker(typename T::Param param) : m_param(std::move(param)) {}

  [[nodiscard]] std::unique_ptr<IPhase> make() const override {
    return std::make_unique<T>(m_param);
  }

 private:
  typename T::Param m_param;
};

/// @brief 型 T に対する PhaseLoader を生成するヘルパー
/// @param parse TOML 値から T::Param を生成するラムダ（失敗時は throw する）
template <PhaseWithParam T, typename F>
PhaseLoader MakeLoader(F&& parse) {
  return [p = std::forward<F>(parse)](const TOMLValue& step) -> LoadedPhase {
    auto param = p(step);
    Optional<String> ref = none;
    if constexpr (SectionRefParam<typename T::Param>) {
      ref = param.sectionName;
    }
    return LoadedPhase{
        .maker = std::make_shared<const PhaseMaker<T>>(std::move(param)),
        .referencedSection = std::move(ref),
    };
  };
}

}  // namespace

const PhaseLoaderTable& GetPhaseLoaders() {
  static const PhaseLoaderTable loaders{
      {U"test_menu",
       MakeLoader<TestMenuPhase>([](const TOMLValue&) -> TestMenuPhase::Param {
         return TestMenuPhase::Param{};
       })},
      {U"title",
       MakeLoader<TitlePhase>([](const TOMLValue&) -> TitlePhase::Param {
         return TitlePhase::Param{};
       })},
      {U"animation_viewer",
       MakeLoader<AnimationViewerPhase>(
           [](const TOMLValue& step) -> AnimationViewerPhase::Param {
             return AnimationViewerPhase::Param{
                 .dataKey = TomlTable{step}.get<String>(U"param")
             };
           }
       )},
      {U"stage",
       MakeLoader<StagePhase>([](const TOMLValue& step) -> StagePhase::Param {
         return StagePhase::Param{
             .stageName = TomlTable{step}.get<String>(U"param")
         };
       })},
      {U"scenario",
       MakeLoader<ScenarioPhase>(
           [](const TOMLValue& step) -> ScenarioPhase::Param {
             return ScenarioPhase::Param{
                 .sectionName = TomlTable{step}.get<String>(U"param")
             };
           }
       )},
      {U"wait",
       MakeLoader<WaitPhase>([](const TOMLValue& step) -> WaitPhase::Param {
         return WaitPhase::Param{
             .duration = TomlTable{step}.get<double>(U"duration")
         };
       })},
  };
  return loaders;
}
