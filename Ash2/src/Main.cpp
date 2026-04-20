#include <Siv3D.hpp>

#include <ThirdParty/entt/entt.hpp>

#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Input/PlayerInputAction.hpp"
#include "Phase/FrameData.hpp"
#include "Phase/PhaseRegistry.hpp"
#include "Phase/PhaseStack.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/DrawSystem.hpp"
#include "System/NameLookup.hpp"

#if USE_TEST
#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>

static void RunTests() {
  Console.open();
  if (Catch::Session().run() != 0) {
    static_cast<void>(std::getchar());
  }
}
#endif

void Main() {
#if USE_TEST
  RunTests();
#endif

  entt::registry registry;
  registry.ctx().emplace<NameLookup>();

  const TOMLReader playerToml(U"config/player.toml");
  registry.ctx().emplace<PlayerConfig>(PlayerConfig::FromToml(playerToml));

  const TOMLReader scenarioToml(U"config/scenario.toml");
  registry.ctx().emplace<ScenarioData>(ScenarioData::FromToml(scenarioToml));

  registry.ctx().emplace<PhaseRegistry>(MakeDefaultPhaseRegistry());

  const PlayerInputAction actions = PlayerInputAction::Default();
  PhaseStack phaseStack(std::make_unique<ScenarioPhase>(U"init"), registry);

  while (System::Update()) {
    const FrameData frameData{
        .dt = Scene::DeltaTime(),
        .input = actions.toInputState(),
    };
    phaseStack.update(registry, frameData);
    AttachmentSystem::UpdateTransform(registry);
    DrawSystem::Draw(registry);
  }
}
