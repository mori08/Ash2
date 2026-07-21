#include <Siv3D.hpp>

#include <functional>

#include "Component/ReactionLevel.hpp"
#include "Component/Stamina.hpp"
#include "Phase/FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

/// @brief 突き出し軌道（近接1・3段目）の珠オフセット（プレイヤー相対）を返す
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
Vec3 MeleeThrustOffset(double progress, bool facingRight, double reach,
                       double capMidH) {
  const double sign = facingRight ? 1.0 : -1.0;
  const double eased = EaseOutQuad(Clamp(progress, 0.0, 1.0));
  return Vec3{sign * reach * eased, capMidH, 0.0};
}

/// @brief 斬り上げ軌道（近接2段目）の珠オフセットを返す
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
// reach / capMidH は隣接する double 引数として渡すと取り違えやすいため
// （bugprone-easily-swappable-parameters 対策）、両者を保持する MeleeConfig
// への const 参照で受け取る。
Vec3 MeleeSlashOffset(double progress, bool facingRight,
                      const MeleeConfig& melee, double slashRiseHeight) {
  const double sign = facingRight ? 1.0 : -1.0;
  const double eased = EaseOutQuad(Clamp(progress, 0.0, 1.0));
  const double horizontal = sign * melee.reach * eased;
  const double vertical = melee.capMidH + (eased - 0.5) * slashRiseHeight;
  return Vec3{horizontal, vertical, 0.0};
}

/// @brief 段の軌道設定から、進行度→珠オフセットのラムダを作る
std::function<Vec3(double)> MakeMeleeOffsetFn(const MeleeStageConfig& stage,
                                              bool facingRight,
                                              const MeleeConfig& melee) {
  switch (stage.trajectory) {
    case MeleeTrajectory::Slash:
      return [=, &melee](double progress) {
        return MeleeSlashOffset(progress, facingRight, melee,
                                stage.slashRiseHeight);
      };
    case MeleeTrajectory::Thrust:
    default:
      return [=, &melee](double progress) {
        return MeleeThrustOffset(progress, facingRight, melee.reach,
                                 melee.capMidH);
      };
  }
}

/// @brief クリップ名が同じでも再生位置を先頭へ強制的に戻す
void RestartClip(SpriteAnimation& anim, const String& clip) {
  anim.currentClip = clip;
  anim.elapsed = 0.0;
}

}  // namespace

Melee MakeMelee(SpriteAnimation& anim, size_t stage,
                entt::entity hitboxEntity) {
  // 段ごとに専用クリップ（melee_1/melee_2/melee_3）へ切り替わるため、
  // 呼び出し元（初回入場・コンボ継続いずれも）で必ずクリップ名が変わり、
  // RestartClip と SetClip の挙動差は生じない。
  RestartClip(anim, U"melee_{}"_fmt(stage + 1));
  return Melee{.stage = stage, .elapsed = 0.0, .hitboxEntity = hitboxEntity};
}

Optional<Motion> Tick(Melee& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& melee = cfg.melee;
  const auto& stageCfg = melee.stages[state.stage];
  const auto& timeline = stageCfg.timeline;
  const auto& input = frameData.input;
  auto& anim = registry.get<SpriteAnimation>(entity);

  state.elapsed += frameData.dt;

  const bool hasNextStage = state.stage + 1 < melee.stages.size();
  // 次段を持つ1・2段目は小さくひるむだけ、締め技（最終段）は吹っ飛ばす
  const ReactionLevel reaction =
      hasNextStage ? ReactionLevel::Stagger : ReactionLevel::Blow;

  const auto offsetFn = MakeMeleeOffsetFn(stageCfg, anim.facingRight, melee);
  UpdateAttackHitbox(registry, entity, state.elapsed, timeline, stageCfg.radius,
                     melee.damage, reaction, state.hitboxEntity, offsetFn);

  if (hasNextStage) {
    // 構え〜後隙A中の攻撃入力は次段への遷移を予約するのみ
    if (!timeline.isCancelable(state.elapsed) && input.attackDown) {
      state.comboQueued = true;
    }

    // 後隙Bに入った時点で予約済み、または後隙B中の新規入力があれば即座に次段へ
    if (timeline.isCancelable(state.elapsed) &&
        (state.comboQueued || input.attackDown)) {
      return MakeMelee(anim, state.stage + 1);
    }

    // 後隙中のダッシュ入力でダッシュへキャンセル（ST不足時は無視）
    // 締め技（最終段）はキャンセル不可のためこの分岐に入らない
    if (timeline.isCancelable(state.elapsed) && input.dashDown &&
        registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
      return MakeDash(registry, entity, cfg, anim, /*air=*/false);
    }
  }

  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
