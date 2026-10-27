#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Config/InputConfig.hpp"
#include "Config/ScenarioData.hpp"
#include "DebugOnly.hpp"
#include "FrameData.hpp"
#include "GameSetup.hpp"
#include "Input/InputDeviceSelector.hpp"
#include "Phase/PhaseStack.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/DrawSystem.hpp"
#include "System/GaugeSystem.hpp"
#include "System/HudSystem.hpp"

namespace {

/// @brief ウィンドウが閉じられるまでフレームの更新と描画を繰り返す
void RunGameLoop(
    entt::registry& registry, InputDeviceSelector& inputSelector,
    PhaseStack& phaseStack
) {
  while (System::Update()) {
    const FrameData frameData{
        .dt = Scene::DeltaTime(),
        .input = inputSelector.update(registry.ctx().get<InputConfig>()),
    };
    DebugOnly::UpdateConfigReload(registry);
    phaseStack.update(registry, frameData);
    GaugeSystem::Update(registry);
    AttachmentSystem::UpdateTransform(registry);

    DrawSystem::Draw(registry);
    DebugOnly::DrawColliders(registry);
    HudSystem::Draw(registry);
  }
}

/// @brief ゲームに必要な状態を構築し、ゲームループへ渡す
void Run() {
  InitializeEngine();

  entt::registry registry;
  InitializeRegistry(registry);

  InputDeviceSelector inputSelector;

  PhaseStack phaseStack(
      std::make_unique<ScenarioPhase>(
          ScenarioPhase::Param{.sectionName = String{kInitSectionName}}
      ),
      registry
  );

  RunGameLoop(registry, inputSelector, phaseStack);
}

}  // namespace

void Main() {
  DebugOnly::RunTestsIfRequested();
  DebugOnly::OpenDebugConsole();

  try {
    Run();
  } catch (...) {  // NOLINT(bugprone-empty-catch)
    // Siv3D に任せると what() の内容がダイアログに出るため、受け止めて終える
    // 記録はデバッガが投げた時点で取っている
  }
}
