#include <Siv3D.hpp>

#include "Component/Invincible.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

Dash MakeDash(
    entt::registry& registry, entt::entity entity, const PlayerConfig& cfg,
    SpriteAnimation& anim, bool air
) {
  auto& stamina = registry.get<Stamina>(entity);
  stamina.current = Max(0, stamina.current - cfg.dash.staminaCost);
  SetClip(anim, U"dash");
  return Dash{.air = air};
}

Optional<Motion> Tick(
    Dash& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& dash = cfg.dash;
  const auto& timeline = dash.timeline;
  const auto& input = frameData.input;
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  state.elapsed += frameData.dt;

  // 接地検出はタイマー満了より先に評価する（空中発動時のみ、地上 Dash
  // は接地遷移を持たない）
  if (state.air && pos.isOnGround()) {
    registry.remove<Invincible>(entity);
    return Landing{.timer = cfg.landing.recoverySec};
  }

  // 構え・ダッシュ中（後隙入り前）は無敵、後隙入りで除去する
  if (state.elapsed < timeline.activeEnd()) {
    registry.emplace_or_replace<Invincible>(entity);
  } else {
    registry.remove<Invincible>(entity);
  }

  // ダッシュ移動中：フリー方向、無方向なら facingRight から前方
  // 空中発動時は移動区間中の垂直速度を 0 に固定する（重力の影響を受けない、
  // 暫定仕様）。移動中に方向ベクトルを lastDashDir へ記録する
  // （後隙では vel がゼロになるため後隙より前に必ず記録を終える）
  if (timeline.isActive(state.elapsed)) {
    if (state.air) vel.h = 0.0;
    if (!input.moveAxis.isZero()) {
      const Vec2 dir = input.moveAxis.normalized();
      vel.w = dir.x * dash.speed;
      vel.d = dir.y * dash.speed;
      state.lastDashDir = dir;
    } else {
      const double sign = anim.facingRight ? 1.0 : -1.0;
      vel.w = sign * dash.speed;
      vel.d = 0.0;
      state.lastDashDir = Vec2{sign, 0.0};
    }
  } else {
    vel.w = 0.0;
    vel.d = 0.0;
  }

  // 後隙B終了時：常に Neutral へ戻る（DashAttack 遷移より先に評価する）
  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  // 後隙B中：前フレームまでに予約済みならダッシュ攻撃へ遷移
  // 入力受付より先に評価することで、今フレームの入力は次フレーム以降に発動する
  if (timeline.isCancelable(state.elapsed) && state.dashAttackQueued) {
    return MakeDashAttack(anim, state.air, state.lastDashDir);
  }

  // ダッシュ中・後隙A・B中の入力で遷移を予約する（DashAttack チェック後に評価）
  if (state.elapsed >= timeline.activeStart() && input.attackDown) {
    state.dashAttackQueued = true;
  }
  if (state.elapsed >= timeline.activeStart() && input.dashDown) {
    state.dashQueued = true;
  }

  // 後隙B中：再ダッシュ予約済みならダッシュへキャンセル（スタミナ不足時は無視）
  if (timeline.isCancelable(state.elapsed) && state.dashQueued &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    return MakeDash(registry, entity, cfg, anim, state.air);
  }

  return none;
}

}  // namespace PlayerMotion
