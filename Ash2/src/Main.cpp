#include <Siv3D.hpp>

#include <ThirdParty/entt/entt.hpp>

#include "Config/PlayerConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "Input/PlayerInputAction.hpp"
#include "Scene/PhaseStack.hpp"
#include "Scene/ScenarioPhase.hpp"
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
  registry.ctx().emplace<PlayerInputAction>(PlayerInputAction::Default());

  const TOMLReader scenarioToml(U"config/scenario.toml");
  registry.ctx().emplace<ScenarioData>(ScenarioData::FromToml(scenarioToml));

  PhaseStack phaseStack(std::make_unique<ScenarioPhase>(U"init"), registry);

  while (System::Update()) {
    phaseStack.update(registry);
    DrawSystem::Draw(registry);
  }
}
