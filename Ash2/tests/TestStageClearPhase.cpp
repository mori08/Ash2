#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Config/AnimationData.hpp"
#include "FrameData.hpp"
#include "Phase/StageClearPhase.hpp"
#include "Phase/TestMenuPhase.hpp"

namespace {
/// @brief AnimationSystem が ctx から取得するため、空のレジストリを積む
[[nodiscard]] entt::registry MakeRegistry() {
  entt::registry registry;
  registry.ctx().emplace<AnimationDataRegistry>();
  return registry;
}
}  // namespace

TEST_CASE("StageClearPhase::update - returns None when dt is zero") {
  auto registry = MakeRegistry();
  StageClearPhase phase;
  auto cmd = phase.update(registry, FrameData{.dt = 0.0});
  REQUIRE(std::holds_alternative<PhaseCommand::None>(cmd.value()));
}

TEST_CASE(
    "StageClearPhase::update - returns Reset to TestMenuPhase after enough "
    "dt"
) {
  auto registry = MakeRegistry();
  StageClearPhase phase;
  auto cmd = phase.update(registry, FrameData{.dt = 100.0});
  REQUIRE(std::holds_alternative<PhaseCommand::Reset>(cmd.value()));
  const auto& next = std::get<PhaseCommand::Reset>(cmd.value()).nextPhase;
  REQUIRE(dynamic_cast<TestMenuPhase*>(next.get()) != nullptr);
}

TEST_CASE(
    "StageClearPhase::update - accumulated small dt eventually returns Reset"
) {
  auto registry = MakeRegistry();
  StageClearPhase phase;
  PhaseCommand cmd = PhaseCommand::None{};
  for (int32 i = 0; i < 1000; ++i) {
    cmd = phase.update(registry, FrameData{.dt = 0.1});
    if (std::holds_alternative<PhaseCommand::Reset>(cmd.value())) {
      break;
    }
  }
  REQUIRE(std::holds_alternative<PhaseCommand::Reset>(cmd.value()));
}

#endif
