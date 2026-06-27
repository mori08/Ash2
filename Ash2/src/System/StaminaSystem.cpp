#include "System/StaminaSystem.hpp"

#include "Component/Motion.hpp"
#include "Component/Player.hpp"
#include "Component/PlayerMotion.hpp"
#include "Component/Stamina.hpp"
#include "Config/PlayerConfig.hpp"

namespace {
// 99% 以上を満タンとみなして丸める閾値（浮動小数点誤差が残り続けるのを防ぐ）
constexpr double KFullThreshold = 0.99;
}  // namespace

void StaminaSystem::Update(entt::registry& registry, double dt) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();

  auto view = registry.view<Player, Stamina, Motion>();
  for (auto&& [entity, stamina, motion] : view.each()) {
    // Neutral 以外の状態はタイマーをリセットして回復しない
    if (!std::holds_alternative<PlayerMotion::Neutral>(motion)) {
      stamina.recoveryTimer = 0.0;
      continue;
    }

    // Neutral
    // が継続している間だけタイマーを進め、待機時間を満たすまで回復しない
    stamina.recoveryTimer += dt;
    if (stamina.recoveryTimer < cfg.stamina.recoveryDelay) continue;

    if (stamina.current >= stamina.max) continue;

    // 残量が少ないほど回復が速い（満タン付近でゼロに近づく）
    const double gain =
        (stamina.max - stamina.current) * cfg.stamina.recoveryRate * dt;

    // 端数を accum に積み立てて毎フレームの切り捨て誤差を防ぐ
    stamina.accum += gain;
    const int intGain = static_cast<int>(stamina.accum);
    if (intGain > 0) {
      stamina.accum -= static_cast<double>(intGain);
      stamina.current += intGain;
    }

    // 満タンに近い残量を丸めて微小な誤差が残り続けるのを防ぐ
    if (stamina.current >= static_cast<int>(stamina.max * KFullThreshold)) {
      stamina.current = stamina.max;
    }
  }
}
