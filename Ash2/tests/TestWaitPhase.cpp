#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Phase/FrameData.hpp"
#include "Phase/WaitPhase.hpp"

TEST_CASE("WaitPhase - returns None before duration elapses") {
  entt::registry registry;
  WaitPhase phase{WaitPhase::Param{.duration = 2.0}};
  auto cmd = phase.update(registry, FrameData{.dt = 1.0});
  REQUIRE(std::holds_alternative<PhaseCommand::None>(cmd.value()));
}

TEST_CASE("WaitPhase - returns Pop when duration is reached exactly") {
  entt::registry registry;
  WaitPhase phase{WaitPhase::Param{.duration = 1.0}};
  auto cmd = phase.update(registry, FrameData{.dt = 1.0});
  REQUIRE(std::holds_alternative<PhaseCommand::Pop>(cmd.value()));
}

TEST_CASE("WaitPhase - returns Pop after accumulated dt exceeds duration") {
  entt::registry registry;
  WaitPhase phase{WaitPhase::Param{.duration = 1.0}};
  auto first = phase.update(registry, FrameData{.dt = 0.6});
  REQUIRE(std::holds_alternative<PhaseCommand::None>(first.value()));
  auto second = phase.update(registry, FrameData{.dt = 0.6});
  REQUIRE(std::holds_alternative<PhaseCommand::Pop>(second.value()));
}

#endif
