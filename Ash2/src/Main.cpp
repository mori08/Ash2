#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Asset.hpp"
#include "Debug.hpp"
#include "GameSetup.hpp"
#include "Input/InputDeviceSelector.hpp"
#include "Phase/FrameData.hpp"
#include "Phase/PhaseStack.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/DrawSystem.hpp"
#include "System/HudSystem.hpp"

#ifdef _DEBUG
#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>

static int32 RunTests() { return Catch::Session().run(); }
#endif

void Main() {
#ifdef _DEBUG
  size_t envLen = 0;
  if (getenv_s(&envLen, nullptr, 0, "ASH2_RUN_TESTS") == 0 && envLen > 0) {
    AppDebug::testMode = true;
    const int32 result = RunTests();
    System::Exit();
    return;
  }
  Console.open();
  APP_LOG(U"=== Debug Build ===");
#endif

  try {
    RegisterAssets();

    entt::registry registry;
    InitializeRegistry(registry);

    InputDeviceSelector inputSelector;

    PhaseStack phaseStack(std::make_unique<ScenarioPhase>(
                              ScenarioPhase::Param{.sectionName = U"init"}),
                          registry);

    while (System::Update()) {
      const FrameData frameData{
          .dt = Scene::DeltaTime(),
          .input = inputSelector.update(),
      };
#ifdef _DEBUG
      if (frameData.input.reloadConfig) {
        ReloadConfig(registry);
      }
#endif
      phaseStack.update(registry, frameData);
      AttachmentSystem::UpdateTransform(registry);
      DrawSystem::Draw(registry);
      HudSystem::Draw(registry);
    }

  } catch (const std::exception& e) {
    const String message = U"[例外] " + Unicode::Widen(e.what());
    TextWriter{U"crash.log", OpenMode::Append}.writeln(message);
    APP_LOG(message);
    throw;
  }
}
