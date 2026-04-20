#if USE_TEST
#include <ThirdParty/Catch2/catch.hpp>
#include <ThirdParty/entt/entt.hpp>

#include "Phase/FrameData.hpp"
#include "Phase/WaitPhase.hpp"

TEST_CASE("WaitPhase - returns None before duration elapses") {
  entt::registry registry;
  WaitPhase phase{2.0};
  auto cmd = phase.update(registry, FrameData{.dt = 1.0});
  REQUIRE(cmd.type == IPhase::PhaseCommand::Type::None);
}

TEST_CASE("WaitPhase - returns Pop when duration is reached exactly") {
  entt::registry registry;
  WaitPhase phase{1.0};
  auto cmd = phase.update(registry, FrameData{.dt = 1.0});
  REQUIRE(cmd.type == IPhase::PhaseCommand::Type::Pop);
}

TEST_CASE("WaitPhase - returns Pop after accumulated dt exceeds duration") {
  entt::registry registry;
  WaitPhase phase{1.0};
  auto first = phase.update(registry, FrameData{.dt = 0.6});
  REQUIRE(first.type == IPhase::PhaseCommand::Type::None);
  auto second = phase.update(registry, FrameData{.dt = 0.6});
  REQUIRE(second.type == IPhase::PhaseCommand::Type::Pop);
}

#endif
