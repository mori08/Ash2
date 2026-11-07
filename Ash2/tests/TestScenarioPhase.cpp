#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include <entt/entt.hpp>

#include "Config/ScenarioData.hpp"
#include "FrameData.hpp"
#include "Phase/PhaseLoaders.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "Phase/WaitPhase.hpp"

namespace {

/// @brief TOML 文字列からシナリオを読み、registry.ctx() へ登録する
void SetupScenario(entt::registry& registry, std::string_view toml) {
  const TOMLReader reader{MemoryViewReader{toml.data(), toml.size()}};
  registry.ctx().emplace<ScenarioData>(
      ScenarioData::FromToml(reader, GetPhaseLoaders())
  );
}

/// @brief 積まれる予定のフェーズが WaitPhase か
bool IsWaitPhase(const std::unique_ptr<IPhase>& phase) {
  return dynamic_cast<WaitPhase*>(phase.get()) != nullptr;
}

}  // namespace

TEST_CASE(
    "ScenarioPhase - runs one step per update and pops when the steps run "
    "out"
) {
  entt::registry registry;
  SetupScenario(
      registry,
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.0\n"
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 2.0\n"
  );
  ScenarioPhase phase{ScenarioPhase::Param{.sectionName = U"intro"}};
  phase.onAfterPush(registry);

  // 1ステップ目・2ステップ目はそれぞれのフェーズを積む
  for (int32 i = 0; i < 2; ++i) {
    auto cmd = phase.update(registry, FrameData{});
    auto* push = std::get_if<PhaseCommand::Push>(&cmd.value());
    REQUIRE(push != nullptr);
    REQUIRE(IsWaitPhase(push->nextPhase));
  }

  // ステップが尽きたら自身を取り出す
  const auto last = phase.update(registry, FrameData{});
  REQUIRE(std::holds_alternative<PhaseCommand::Pop>(last.value()));
}

TEST_CASE("ScenarioPhase - a reset step returns Reset") {
  entt::registry registry;
  SetupScenario(
      registry,
      "[[intro]]\n"
      "action = \"reset\"\n"
      "phase = \"wait\"\n"
      "duration = 1.0\n"
  );
  ScenarioPhase phase{ScenarioPhase::Param{.sectionName = U"intro"}};
  phase.onAfterPush(registry);

  auto cmd = phase.update(registry, FrameData{});
  auto* reset = std::get_if<PhaseCommand::Reset>(&cmd.value());
  REQUIRE(reset != nullptr);
  REQUIRE(IsWaitPhase(reset->nextPhase));
}

TEST_CASE("ScenarioPhase - restarts from the first step when pushed again") {
  entt::registry registry;
  SetupScenario(
      registry,
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.0\n"
  );
  ScenarioPhase phase{ScenarioPhase::Param{.sectionName = U"intro"}};
  phase.onAfterPush(registry);
  (void)phase.update(registry, FrameData{});

  phase.onAfterPush(registry);

  const auto cmd = phase.update(registry, FrameData{});
  REQUIRE(std::holds_alternative<PhaseCommand::Push>(cmd.value()));
}

TEST_CASE("ScenarioPhase - throws for a section missing from ScenarioData") {
  entt::registry registry;
  SetupScenario(
      registry,
      "[[intro]]\n"
      "action = \"push\"\n"
      "phase = \"wait\"\n"
      "duration = 1.0\n"
  );
  ScenarioPhase phase{ScenarioPhase::Param{.sectionName = U"unknown"}};
  phase.onAfterPush(registry);

  REQUIRE_THROWS_AS((void)phase.update(registry, FrameData{}), Error);
}

#endif
