#pragma once

#include "IPhase.hpp"

/// @brief 指定秒数待機してから Pop するフェーズ
class WaitPhase : public IPhase {
 public:
  /// @brief コンストラクタ
  /// @param duration 待機時間（秒）
  explicit WaitPhase(double duration);

  /// @brief 経過時間を積算し、duration を超えたら Pop を返す
  /// @param registry ECS レジストリ
  /// @param dt 経過時間（秒）
  /// @return duration 経過後に Pop、それまでは None
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    double dt) override;

 private:
  /// 待機時間（秒）
  double m_duration;
  /// 積算経過時間（秒）
  double m_elapsed = 0.0;
};
