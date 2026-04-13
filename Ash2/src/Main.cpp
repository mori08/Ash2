#include <Siv3D.hpp>

#include <ThirdParty/entt/entt.hpp>

#include "Config/PlayerConfig.hpp"
#include "Input/PlayerInputAction.hpp"
#include "Scene/DemoPhase.hpp"
#include "Scene/PhaseStack.hpp"
#include "System/DrawSystem.hpp"

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

  const TOMLReader toml(U"config/player.toml");
  registry.ctx().emplace<PlayerConfig>(PlayerConfig::FromToml(toml));
  registry.ctx().emplace<PlayerInputAction>(PlayerInputAction::Default());

  PhaseStack phaseStack(std::make_unique<DemoPhase>(), registry);

  while (System::Update()) {
    phaseStack.update(registry);
    DrawSystem::draw(registry);
  }
}
