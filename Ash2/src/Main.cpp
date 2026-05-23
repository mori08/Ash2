#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Asset.hpp"
#include "GameSetup.hpp"
#include "Input/PlayerInputAction.hpp"
#include "Phase/FrameData.hpp"
#include "Phase/PhaseStack.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/AttachmentSystem.hpp"
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
#ifdef _DEBUG
  Console.open();
  Console << U"=== Debug Build ===";
#endif

  try {
    RegisterAssets();

#if USE_TEST
    RunTests();
#endif

    entt::registry registry;
    InitializeRegistry(registry);

    const PlayerInputAction actions = PlayerInputAction::Default();
    PhaseStack phaseStack(std::make_unique<ScenarioPhase>(U"init"), registry);

    while (System::Update()) {
      const FrameData frameData{
          .dt = Scene::DeltaTime(),
          .input = actions.toInputState(),
      };
#ifdef _DEBUG
      if (frameData.input.reloadConfig) {
        ReloadConfig(registry);
      }
#endif
      phaseStack.update(registry, frameData);
      AttachmentSystem::UpdateTransform(registry);
      DrawSystem::Draw(registry);
    }

  } catch (const std::exception& e) {
#ifdef _DEBUG
    Console << U"[例外] " << Unicode::Widen(e.what());
    Console << U"Enterキーで終了...";
    static_cast<void>(std::getchar());
#endif
    throw;
  }
}
