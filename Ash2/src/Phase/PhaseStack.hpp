#pragma once

#include "IPhase.hpp"
#include "Phase/FrameData.hpp"

/// @brief フェーズをスタックで管理するクラス
class PhaseStack {
 public:
  /// @brief コンストラクタ
  /// @param initialPhase 初期フェーズ
  /// @param registry ECS レジストリ
  explicit PhaseStack(std::unique_ptr<IPhase>&& initialPhase,
                      entt::registry& registry);

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @param frameData フレームごとの更新データ
  void update(entt::registry& registry, const FrameData& frameData);

 private:
  /// @brief スタックの先頭フェーズを取り出す
  /// @param registry ECS レジストリ
  void pop(entt::registry& registry);

  /// @brief スタックにフェーズを積む
  /// @param registry ECS レジストリ
  /// @param phase 積むフェーズ
  void push(entt::registry& registry, std::unique_ptr<IPhase>&& phase);

  /// スタック（末尾が最前面）
  Array<std::unique_ptr<IPhase>> m_stack;
};
