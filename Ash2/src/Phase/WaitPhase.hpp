#pragma once

#include "IPhase.hpp"

/// @brief 指定秒数待機してから Pop するフェーズ
class WaitPhase : public IPhase {
 public:
  /// @brief WaitPhase の生成パラメータ
  struct Param {
    /// 待機時間（秒）
    double duration;
  };

  explicit WaitPhase(const Param& param);

  /// @brief 経過時間を積算し、duration を超えたら Pop を返す
  /// @return duration 経過後に Pop、それまでは None
  [[nodiscard]] PhaseCommand update(
      entt::registry& registry, const FrameData& frameData
  ) override;

 private:
  /// 待機時間（秒）
  double m_duration;
  /// 積算経過時間（秒）
  double m_elapsed = 0.0;
};
