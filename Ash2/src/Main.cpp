#include <Siv3D.hpp>

#include <cstdlib>
#include <entt/entt.hpp>
#include <iostream>

#include "Asset.hpp"
#include "Config/ScenarioData.hpp"
#include "CrashHandler.hpp"
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

namespace {

/// @brief アセットの登録からゲームループの終了までを行う
void Run() {
  if (auto result = RegisterAssets(); !result) {
    throw FatalError{
        .reason = FatalReason::AssetMissing,
        .detail = std::move(result).error(),
    };
  }
  Scene::SetTextureFilter(TextureFilter::Nearest);

  entt::registry registry;
  if (auto result = InitializeRegistry(registry); !result) {
    throw FatalError{
        .reason = FatalReason::ConfigInvalid,
        .detail = std::move(result).error(),
    };
  }

  InputDeviceSelector inputSelector;

  PhaseStack phaseStack(
      std::make_unique<ScenarioPhase>(
          ScenarioPhase::Param{.sectionName = String{kInitSectionName}}
      ),
      registry
  );

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
}

}  // namespace

void Main() {
#ifdef _DEBUG
  size_t envLen = 0;
  if (getenv_s(&envLen, nullptr, 0, "ASH2_RUN_TESTS") == 0 && envLen > 0) {
    AppDebug::testMode = true;
    const int32 result = RunTests();
    // std::exit() は s3d のスレッドと噛み合って終了しない
    // _Exit() は出力を流さないため、先に明示的に流す
    // TODO(#278): 終了コードを設定する手段がなく _Exit に頼っている
    std::cout.flush();
    std::_Exit(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
  }
  Console.open();
  APP_LOG(U"=== Debug Build ===");
#endif

  try {
    Run();
  } catch (const FatalError& e) {
    ExitWithFatal(e);
  } catch (const Error& e) {
    ExitWithFatal({
        .reason = FatalReason::Unknown,
        .detail = U"{}: {}"_fmt(e.type(), e.what()),
    });
  } catch (const std::exception& e) {
    ExitWithFatal({
        .reason = FatalReason::Unknown,
        .detail = Unicode::Widen(e.what()),
    });
  } catch (...) {
    ExitWithFatal({
        .reason = FatalReason::Unknown,
        .detail = U"未知の例外",
    });
  }
}
