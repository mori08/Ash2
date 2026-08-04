#include "Phase/PhaseLoaders.hpp"

#include "Phase/AnimationViewerPhase.hpp"
#include "Phase/PlayerTestPhase.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/TestMenuPhase.hpp"
#include "Phase/WaitPhase.hpp"

namespace {

/// @brief IPhase を継承し、const Param& から構築可能な Param を持つフェーズ型
template <typename T>
concept PhaseWithParam = std::derived_from<T, IPhase> &&
                         std::move_constructible<typename T::Param> &&
                         std::constructible_from<T, const typename T::Param&>;

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
/// @param parse TOML 値から T::Param を生成するラムダ（失敗時は
/// unexpected でエラーメッセージを返す）
template <PhaseWithParam T, typename F>
PhaseLoader MakeLoader(F&& parse) {
  return [p = std::forward<F>(parse)](const TOMLValue& step
         ) -> std::expected<IPhaseMaker::Ptr, String> {
    auto param = p(step);
    if (!param) {
      return std::unexpected{std::move(param).error()};
    }
    return std::make_shared<const PhaseMaker<T>>(*std::move(param));
  };
}

}  // namespace

const PhaseLoaderTable& GetPhaseLoaders() {
  static const PhaseLoaderTable loaders{
      {U"player_test",
       MakeLoader<PlayerTestPhase>(
           [](const TOMLValue&)
               -> std::expected<PlayerTestPhase::Param, String> {
             return PlayerTestPhase::Param{};
           }
       )},
      {U"test_menu",
       MakeLoader<TestMenuPhase>(
           [](const TOMLValue&) -> std::expected<TestMenuPhase::Param, String> {
             return TestMenuPhase::Param{};
           }
       )},
      {U"animation_viewer",
       MakeLoader<AnimationViewerPhase>(
           [](const TOMLValue& step
           ) -> std::expected<AnimationViewerPhase::Param, String> {
             const auto dataKey = step[U"param"].getOpt<String>();
             if (!dataKey) {
               return std::unexpected{
                   U"ScenarioData::ParseStep: animation_viewer に param があ"
                   U"りません"
               };
             }
             return AnimationViewerPhase::Param{.dataKey = *dataKey};
           }
       )},
      {U"scenario",
       MakeLoader<ScenarioPhase>(
           [](const TOMLValue& step
           ) -> std::expected<ScenarioPhase::Param, String> {
             const auto sectionName = step[U"param"].getOpt<String>();
             if (!sectionName) {
               return std::unexpected{
                   U"ScenarioData::ParseStep: scenario に param がありません"
               };
             }
             return ScenarioPhase::Param{.sectionName = *sectionName};
           }
       )},
      {U"wait",
       MakeLoader<WaitPhase>(
           [](const TOMLValue& step
           ) -> std::expected<WaitPhase::Param, String> {
             const auto duration = step[U"duration"].getOpt<double>();
             if (!duration) {
               return std::unexpected{
                   U"ScenarioData::ParseStep: wait に duration がありません"
               };
             }
             return WaitPhase::Param{.duration = *duration};
           }
       )},
  };
  return loaders;
}
