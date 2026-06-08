#pragma once

#include "IPhase.hpp"
#include "Phase/FrameData.hpp"

/// @brief フェーズをスタックで管理するクラス
class PhaseStack {
 public:
  explicit PhaseStack(std::unique_ptr<IPhase>&& initialPhase,
                      entt::registry& registry);

  void update(entt::registry& registry, const FrameData& frameData);

 private:
  /// @brief スタックの先頭フェーズを取り出す
  void pop(entt::registry& registry);

  /// @brief スタックにフェーズを積む
  void push(entt::registry& registry, std::unique_ptr<IPhase>&& phase);

  /// スタック（末尾が最前面）
  Array<std::unique_ptr<IPhase>> m_stack;
};
