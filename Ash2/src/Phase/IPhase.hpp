#pragma once

#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Phase/FrameData.hpp"

/// @brief フェーズの基底クラス
class IPhase {
 public:
  /// @brief フェーズスタックへの操作を表す構造体
  struct PhaseCommand {
    /// @brief 操作の種類
    enum class Type : uint8 {
      None,
      Pop,
      Push,
      Reset,
    };

    Type type;
    /// 次のフェーズ（Push / Reset 時のみ有効）
    std::unique_ptr<IPhase> nextPhase;

    /// @brief 何もしないコマンドを返す
    [[nodiscard]] static PhaseCommand None();

    /// @brief スタックから取り出すコマンドを返す
    [[nodiscard]] static PhaseCommand Pop();

    /// @brief スタックにフェーズを積むコマンドを返す
    /// @param phase 次のフェーズ（nullptr 不可）
    [[nodiscard]] static PhaseCommand Push(std::unique_ptr<IPhase>&& phase);

    /// @brief スタックをクリアしてフェーズを積むコマンドを返す
    /// @param phase 次のフェーズ（nullptr 不可）
    [[nodiscard]] static PhaseCommand Reset(std::unique_ptr<IPhase>&& phase);
  };

  virtual ~IPhase() = default;

  /// @brief スタックに積まれた直後に呼ばれる
  virtual void onAfterPush(entt::registry&) {}

  [[nodiscard]] virtual PhaseCommand update(entt::registry& registry,
                                            const FrameData& frameData) = 0;

  /// @brief スタックから取り出される直前に呼ばれる
  virtual void onBeforePop(entt::registry&) {}
};

inline IPhase::PhaseCommand IPhase::PhaseCommand::None() {
  return {.type = Type::None, .nextPhase = nullptr};
}

inline IPhase::PhaseCommand IPhase::PhaseCommand::Pop() {
  return {.type = Type::Pop, .nextPhase = nullptr};
}

inline IPhase::PhaseCommand IPhase::PhaseCommand::Push(
    std::unique_ptr<IPhase>&& phase) {
  assert(phase != nullptr);
  return {.type = Type::Push, .nextPhase = std::move(phase)};
}

inline IPhase::PhaseCommand IPhase::PhaseCommand::Reset(
    std::unique_ptr<IPhase>&& phase) {
  assert(phase != nullptr);
  return {.type = Type::Reset, .nextPhase = std::move(phase)};
}
