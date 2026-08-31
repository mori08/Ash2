#include <Siv3D.hpp>

#include <cstdlib>
#include <entt/entt.hpp>
#include <exception>

#include "Asset.hpp"
#include "Config/ScenarioData.hpp"
#include "CrashHandler.hpp"
#include "Debug.hpp"
#include "FrameData.hpp"
#include "GameSetup.hpp"
#include "Input/InputDeviceSelector.hpp"
#include "Phase/PhaseStack.hpp"
#include "Phase/ScenarioPhase.hpp"
#include "System/AttachmentSystem.hpp"
#include "System/DebugDrawSystem.hpp"
#include "System/DrawSystem.hpp"
#include "System/HudSystem.hpp"

#ifdef _DEBUG
#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>
#endif

namespace {

/// @brief 環境変数 ASH2_RUN_TESTS が設定されていればテストを実行して終了する
/// @note リリースビルドでは何もしない
void RunTestsIfRequested() {
#ifdef _DEBUG
  size_t envLen = 0;
  if (getenv_s(&envLen, nullptr, 0, "ASH2_RUN_TESTS") != 0 || envLen == 0) {
    return;
  }
  AppDebug::testMode = true;
  const int32 result = Catch::Session().run();
  ExitImmediately(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
#endif
}

/// @brief デバッグ出力の準備を行う
/// @note リリースビルドでは何もしない
void OpenDebugConsole() {
#ifdef _DEBUG
  Console.open();
  APP_LOG(U"=== Debug Build ===");
#endif
}

/// @brief ウィンドウが閉じられるまでフレームの更新と描画を繰り返す
void RunGameLoop(
    entt::registry& registry, InputDeviceSelector& inputSelector,
    PhaseStack& phaseStack
) {
  while (System::Update()) {
    const FrameData frameData{
        .dt = Scene::DeltaTime(),
        .input = inputSelector.update(),
    };
#ifdef _DEBUG
    if (frameData.input.reloadConfig) {
      ReloadConfig(registry);
    }
    if (frameData.input.toggleDebugDraw) {
      AppDebug::drawColliders = !AppDebug::drawColliders;
    }
#endif
    phaseStack.update(registry, frameData);
    AttachmentSystem::UpdateTransform(registry);

    DrawSystem::Draw(registry);
#ifdef _DEBUG
    if (AppDebug::drawColliders) {
      DebugDrawSystem::DrawColliders(registry);
    }
#endif
    HudSystem::Draw(registry);
  }
}

/// @brief ゲームに必要な状態を構築し、ゲームループへ渡す
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

  RunGameLoop(registry, inputSelector, phaseStack);
}

/// @brief 現在処理中の例外を分類し、記録・表示して終了する
/// @note catch (...) の中からのみ呼ぶ
[[noreturn]] void ExitWithCurrentException() {
  // 型ごとの catch を一箇所に集めるため、処理中の例外を再送出して振り分ける
  try {
    throw;
  } catch (const FatalError& e) {
    ExitWithFatal(e);
  } catch (const Error& e) {
    // s3d::Error は std::exception を継承しないため独立して受ける
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

}  // namespace

void Main() {
  RunTestsIfRequested();
  OpenDebugConsole();

  try {
    Run();
  } catch (...) {
    ExitWithCurrentException();
  }
}
