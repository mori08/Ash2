#include "DebugOnly.hpp"

#ifdef _DEBUG

#include <Siv3D.hpp>

#include <cstdlib>
#include <iostream>

#include "Asset.hpp"
#include "Config/ArenaConfig.hpp"
#include "Config/EnemyConfig.hpp"
#include "Config/InputConfig.hpp"
#include "Config/PlayerConfig.hpp"
#include "Config/TitleConfig.hpp"
#include "Debug.hpp"
#include "Factory/StageData.hpp"
#include "GameSetup.hpp"
#include "System/DebugDrawSystem.hpp"

#define CATCH_CONFIG_RUNNER
#include <ThirdParty/Catch2/catch.hpp>

namespace {

// Siv3D 予約キー（割り当て不可）:
//   F1 = ライセンス表示 / F12・PrintScreen = スクリーンショット
constexpr Input kConfigReloadKey = KeyF5;
constexpr Input kColliderDrawKey = KeyF2;

/// Collider のデバッグ描画を表示中か（既定は非表示）
bool colliderDrawEnabled = false;

/// @brief 設定を再読込する
/// @note 読み込みに失敗すると throw し、ゲームが終了する
void ReloadConfig(entt::registry& registry) {
  auto player = PlayerConfig::FromToml(OpenToml(U"assets/config/player.toml"));
  auto enemy = EnemyConfig::FromToml(OpenToml(U"assets/config/enemy.toml"));
  auto arena = ArenaConfig::FromToml(OpenToml(U"assets/config/arena.toml"));
  auto title = TitleConfig::FromToml(OpenToml(U"assets/config/title.toml"));
  auto input = InputConfig::FromToml(OpenToml(U"assets/config/input.toml"));
  auto stage = StageData::FromToml(OpenToml(U"assets/config/stage.toml"));
  auto anims = LoadAnimations();

  registry.ctx().get<PlayerConfig>() = std::move(player);
  registry.ctx().get<EnemyConfig>() = enemy;
  registry.ctx().get<ArenaConfig>() = arena;
  registry.ctx().get<TitleConfig>() = title;
  registry.ctx().get<InputConfig>() = input;
  registry.ctx().get<StageData>() = std::move(stage);
  registry.ctx().get<AnimationDataRegistry>() = std::move(anims);
}

/// @brief 環境変数を読む。未設定と空文字は区別せず、どちらも空文字を返す
/// @note getenv_s が返す長さは終端文字を含む
String ReadEnv(const char* name) {
  size_t len = 0;
  if (getenv_s(&len, nullptr, 0, name) != 0) {
    throw Error{U"環境変数を読めない: " + Unicode::Widen(name)};
  }
  if (len <= 1) {
    return String{};
  }
  std::string buffer(len, '\0');
  if (getenv_s(&len, buffer.data(), buffer.size(), name) != 0) {
    throw Error{U"環境変数を読めない: " + Unicode::Widen(name)};
  }
  buffer.resize(len - 1);
  return Unicode::FromUTF8(buffer);
}

/// @brief 撮影用の環境変数の解析結果
struct CaptureEnv {
  /// ASH2_START_SECTION の値。未指定なら none
  Optional<String> startSection;
  /// ASH2_CAPTURE_AFTER による撮影の進行。未指定なら none
  Optional<DebugOnly::AutoCaptureProgress> progress;
};

/// @brief 撮影用の環境変数を初回アクセス時に 1 度だけ解析する
/// @throw Error ASH2_CAPTURE_AFTER が不正な場合
CaptureEnv& GetCaptureEnv() {
  static CaptureEnv env = [] {
    CaptureEnv result;
    if (String section = ReadEnv("ASH2_START_SECTION"); !section.isEmpty()) {
      result.startSection = std::move(section);
    }
    if (const auto after =
            DebugOnly::ParseCaptureAfter(ReadEnv("ASH2_CAPTURE_AFTER"))) {
      result.progress.emplace(*after);
    }
    return result;
  }();
  return env;
}

}  // namespace

namespace DebugOnly {

Optional<double> ParseCaptureAfter(StringView text) {
  if (text.isEmpty()) {
    return none;
  }
  const auto value = ParseOpt<double>(text);
  if (!value || !IsFinite(*value) || *value < 0.0) {
    throw Error{
        U"ASH2_CAPTURE_AFTER は 0 以上の数値で指定する: " + String{text}
    };
  }
  return *value;
}

AutoCaptureProgress::AutoCaptureProgress(double captureAfter)
    : m_captureAfter(captureAfter) {}

AutoCaptureAction AutoCaptureProgress::advance(double dt) {
  if (m_exited) {
    return AutoCaptureAction::None;
  }
  // 撮影と同じフレームで終えると保存されない
  // 保存は次の System::Update で行われるため、1 フレーム置いてから終える
  if (m_captured) {
    m_exited = true;
    return AutoCaptureAction::Exit;
  }
  m_elapsed += dt;
  if (m_elapsed >= m_captureAfter) {
    m_captured = true;
    return AutoCaptureAction::Capture;
  }
  return AutoCaptureAction::None;
}

Optional<String> StartSectionOverride() { return GetCaptureEnv().startSection; }

void UpdateAutoCapture(double dt) {
  auto& progress = GetCaptureEnv().progress;
  if (!progress) {
    return;
  }
  switch (progress.value().advance(dt)) {
    case AutoCaptureAction::Capture:
      ScreenCapture::SaveCurrentFrame();
      break;
    case AutoCaptureAction::Exit:
      System::Exit();
      break;
    case AutoCaptureAction::None:
      break;
  }
}

void RunTestsIfRequested() {
  size_t envLen = 0;
  if (getenv_s(&envLen, nullptr, 0, "ASH2_RUN_TESTS") != 0) {
    throw Error{U"環境変数 ASH2_RUN_TESTS を読めない"};
  }
  if (envLen == 0) {
    return;
  }
  AppDebug::testMode = true;
  const int32 result = Catch::Session().run();
  // std::exit() は s3d のスレッドと噛み合って終了しない
  // _Exit() は出力を流さないため、先に明示的に流す
  // TODO(#278): 終了コードを設定する手段がなく _Exit に頼っている
  std::cout.flush();
  std::_Exit(result);
}

void OpenDebugConsole() {
  Console.open();
  APP_LOG(U"=== Debug Build ===");
}

void SetScreenshotDirectory() {
  ScreenCapture::SetScreenshotDirectory(U"screenshots/");
}

void UpdateConfigReload(entt::registry& registry) {
  if (kConfigReloadKey.down()) {
    ReloadConfig(registry);
  }
}

void DrawColliders(const entt::registry& registry) {
  if (kColliderDrawKey.down()) {
    colliderDrawEnabled = !colliderDrawEnabled;
  }
  if (colliderDrawEnabled) {
    DebugDrawSystem::DrawColliders(registry);
    DebugDrawSystem::DrawBoundary(registry);
  }
}

}  // namespace DebugOnly

#endif
