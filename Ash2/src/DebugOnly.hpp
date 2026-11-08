#pragma once

#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief Debug ビルドにのみ存在する機能とそのキー判定
/// @note Release では空の inline 関数になるため、呼び出し側は `#ifdef _DEBUG`
/// で囲まず無条件に呼ぶ。キー割り当ては DebugOnly.cpp 冒頭にまとめてある
namespace DebugOnly {

#ifdef _DEBUG
/// @brief 自動撮影の進行でそのフレームに行うこと
enum class AutoCaptureAction : uint8 { None, Capture, Exit };

/// @brief ASH2_CAPTURE_AFTER の値（秒）を数値に変換する
/// @return 空文字なら none
/// @throw Error 数値でない・負・有限でない場合
[[nodiscard]] Optional<double> ParseCaptureAfter(StringView text);

/// @brief 開始からの経過を数え、撮影と終了の時機を返す
class AutoCaptureProgress {
 public:
  /// @param captureAfter 撮影までの秒数（0 以上）
  explicit AutoCaptureProgress(double captureAfter);

  /// @brief dt を加算して、そのフレームに行うことを返す
  /// @note 経過が captureAfter に達したフレームで Capture を一度だけ返し、
  /// 次のフレームで Exit を返す。Exit の後は常に None を返す
  [[nodiscard]] AutoCaptureAction advance(double dt);

 private:
  /// 撮影までの秒数
  double m_captureAfter;
  /// 開始からの経過秒数
  double m_elapsed = 0.0;
  /// Capture を返し済みか
  bool m_captured = false;
  /// Exit を返し済みか
  bool m_exited = false;
};

/// @brief 環境変数 ASH2_START_SECTION で指定された開始セクション名を返す
/// @return 未指定なら none。既定の開始セクションは呼び出し側が決める
/// @throw Error 環境変数 ASH2_CAPTURE_AFTER
/// が不正な場合（初回アクセスで解析する）
[[nodiscard]] Optional<String> StartSectionOverride();

/// @brief 環境変数 ASH2_CAPTURE_AFTER（秒）が設定されていれば、その時刻に
/// 画面を 1 枚撮影し、次のフレームで終了する。HUD 描画の後に呼ぶ
void UpdateAutoCapture(double dt);

/// @brief 環境変数 ASH2_RUN_TESTS が設定されていればテストを実行して終了する
void RunTestsIfRequested();

/// @brief デバッグ出力の準備を行う
void OpenDebugConsole();

/// @brief スクリーンショットの保存先を App/screenshots/ にする
void SetScreenshotDirectory();

/// @brief 設定リロードキーが押されていれば設定を再読込する
void UpdateConfigReload(entt::registry& registry);

/// @brief Collider 描画トグルキーが押されていれば表示を切り替え、
/// 表示中なら DebugDrawSystem::DrawColliders を呼ぶ
void DrawColliders(const entt::registry& registry);
#else
[[nodiscard]] inline Optional<String> StartSectionOverride() { return none; }
inline void UpdateAutoCapture(double) {}
inline void RunTestsIfRequested() {}
inline void OpenDebugConsole() {}
inline void SetScreenshotDirectory() {}
inline void UpdateConfigReload(entt::registry&) {}
inline void DrawColliders(const entt::registry&) {}
#endif

}  // namespace DebugOnly
