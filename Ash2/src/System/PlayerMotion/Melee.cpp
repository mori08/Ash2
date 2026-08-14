#include <Siv3D.hpp>

#include <cassert>
#include <functional>

#include "Component/ReactionLevel.hpp"
#include "Component/Stamina.hpp"
#include "Phase/FrameData.hpp"
#include "System/PlayerMotion/Helper.hpp"
#include "System/PlayerMotion/Transition.hpp"
#include "System/PlayerMotionSystem.hpp"

namespace PlayerMotion {

namespace {

/// @brief 突き出し軌道（近接1・締め段）の珠オフセット（プレイヤー相対）を返す
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
Vec3 MeleeThrustOffset(
    double progress, bool facingRight, double reach, double capMidH
) {
  const double sign = facingRight ? 1.0 : -1.0;
  const double eased = EaseOutQuad(Clamp(progress, 0.0, 1.0));
  return Vec3{sign * reach * eased, capMidH, 0.0};
}

/// @brief 斬り上げ軌道（近接2段目）の珠オフセットを返す
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
// reach / capMidH や各種の調整値は隣接する double 引数として渡すと取り違え
// やすいため（bugprone-easily-swappable-parameters 対策）、それぞれを保持する
// MeleeConfig / MeleeSwingConfig への const 参照で受け取る。
Vec3 MeleeSlashOffset(
    double progress, bool facingRight, const MeleeConfig& melee,
    const MeleeSwingConfig& swing
) {
  const double sign = facingRight ? 1.0 : -1.0;
  const double eased = EaseOutQuad(Clamp(progress, 0.0, 1.0));
  const double horizontal = sign * melee.reach * eased;
  // 水平と同じ進行度で上下させると軌跡が直線になってしまうため、垂直だけ
  // 進行を遅らせて「前へ出てから跳ね上がる」弧を描かせる。始点・終点は
  // 変えないので、slashCurve が 0 なら従来どおりの直線に戻る。
  const double verticalT = Math::Lerp(eased, eased * eased, swing.slashCurve);
  const double vertical =
      melee.capMidH + (verticalT - 0.5) * swing.slashRiseHeight;
  return Vec3{horizontal, vertical, 0.0};
}

/// @brief 振りの軌道設定から、進行度→珠オフセットのラムダを作る
std::function<Vec3(double)> MakeMeleeOffsetFn(
    const MeleeSwingConfig& swing, bool facingRight, const MeleeConfig& melee
) {
  switch (swing.trajectory) {
    case MeleeTrajectory::Slash:
      return [=, &melee, &swing](double progress) {
        return MeleeSlashOffset(progress, facingRight, melee, swing);
      };
    case MeleeTrajectory::Thrust:
    default:
      return [=, &melee](double progress) {
        return MeleeThrustOffset(
            progress, facingRight, melee.reach, melee.capMidH
        );
      };
  }
}

/// @brief クリップ名が同じでも再生位置を先頭へ強制的に戻す
void RestartClip(SpriteAnimation& anim, const String& clip) {
  anim.currentClip = clip;
  anim.elapsed = 0.0;
}

/// @brief 近接判定用ヒットボックスを1つ更新する
///
/// 生成するヒットボックスは常に不可視になる。
/// @param reaction ヒット成立時に被弾側へ適用するリアクションの強さ
void UpdateMeleeHitbox(
    entt::registry& registry, entt::entity owner, double elapsed,
    const MotionTimeline& timeline, const MeleeConfig& melee,
    const MeleeSwingConfig& swing, double fadeSec, ReactionLevel reaction,
    entt::entity& hitboxEntity, const std::function<Vec3(double)>& offsetFn
) {
  UpdateAttackHitbox(
      registry, owner, elapsed, timeline,
      HitboxSpec{
          .radius = swing.radius,
          .damage = melee.damage,
          .reaction = reaction,
          .hitstopSec = swing.hitstopSec,
          .fadeSec = fadeSec,
          // 近接の見た目は光エンティティが担うため、判定の珠は描画しない
          .drawOrb = false
      },
      hitboxEntity, offsetFn
  );
}

/// @brief 光1つの中心軌道からの h 方向オフセットを返す
/// @param index 光のインデックス（0 始まり、小さいほど下側）
// progress と index は互いに暗黙変換できてしまうため、締め段設定を
// 間に挟んで隣接させない（bugprone-easily-swappable-parameters 対策）。
double MeleeLightSpread(
    double progress, const MeleeFinisherConfig& finisher, int32 index
) {
  assert(finisher.lightCount >= 2 && "締め段の光は2つ以上でなければならない");
  const double t = Clamp(progress, 0.0, 1.0);
  // 中心軌道と同じ EaseOutQuad で閉じると序盤で間隔が詰まりきってしまい、
  // 光が2つに見える時間がほとんど残らない。踏み込む間は開いたままにして、
  // 終盤で一気に閉じさせる。
  const double closed = t * t;
  const double ratio =
      static_cast<double>(index) / (finisher.lightCount - 1) - 0.5;
  return finisher.lightGap * ratio * (1.0 - closed);
}

}  // namespace

MeleeChain MakeMeleeChain(SpriteAnimation& anim, size_t stage) {
  // 段ごとに専用クリップ（melee_1/melee_2/...）へ切り替わるため、
  // 呼び出し元（初回入場・コンボ継続いずれも）で必ずクリップ名が変わり、
  // RestartClip と SetClip の挙動差は生じない。
  RestartClip(anim, U"melee_{}"_fmt(stage + 1));
  return MeleeChain{.stage = stage};
}

MeleeFinisher MakeMeleeFinisher(SpriteAnimation& anim) {
  RestartClip(anim, U"melee_finish");
  return MeleeFinisher{};
}

Optional<Motion> Tick(
    MeleeChain& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& melee = cfg.melee;
  assert(
      state.stage < melee.chain.size() &&
      "state.stage は melee.chain の範囲内でなければならない"
  );
  const auto& swing = melee.chain[state.stage];
  const auto& timeline = swing.timeline;
  const auto& input = frameData.input;
  auto& anim = registry.get<SpriteAnimation>(entity);

  state.elapsed += frameData.dt;

  const auto offsetFn = MakeMeleeOffsetFn(swing, anim.facingRight, melee);
  UpdateMeleeHitbox(
      registry, entity, state.elapsed, timeline, melee, swing,
      cfg.attackEffect.fadeSec, ReactionLevel::Stagger, state.hitboxEntity,
      offsetFn
  );
  UpdateAttackLights(
      registry, entity, state.elapsed, timeline,
      LightSpec{
          .count = 1,
          .radius = swing.radius,
          .fadeSec = cfg.attackEffect.fadeSec
      },
      state.lightEntities, [&offsetFn](double progress, int32 /*index*/) {
        return offsetFn(progress);
      }
  );

  // 構え〜後隙A中の攻撃入力は次段への遷移を予約するのみ
  if (!timeline.isCancelable(state.elapsed) && input.attackDown) {
    state.comboQueued = true;
  }

  // 後隙Bに入った時点で予約済み、または後隙B中の新規入力があれば即座に次段へ
  if (timeline.isCancelable(state.elapsed) &&
      (state.comboQueued || input.attackDown)) {
    return state.stage + 1 < melee.chain.size()
               ? Motion{MakeMeleeChain(anim, state.stage + 1)}
               : Motion{MakeMeleeFinisher(anim)};
  }

  // 後隙中のダッシュ入力でダッシュへキャンセル（ST不足時は無視）
  if (timeline.isCancelable(state.elapsed) && input.dashDown &&
      registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    return MakeDash(registry, entity, cfg, anim, /*air=*/false);
  }

  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

Optional<Motion> Tick(
    MeleeFinisher& state, entt::registry& registry, entt::entity entity,
    const FrameData& frameData
) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& melee = cfg.melee;
  const auto& finisher = melee.finisher;
  const auto& swing = finisher.swing;
  const auto& timeline = swing.timeline;
  auto& anim = registry.get<SpriteAnimation>(entity);

  state.elapsed += frameData.dt;

  const auto offsetFn = MakeMeleeOffsetFn(swing, anim.facingRight, melee);
  UpdateMeleeHitbox(
      registry, entity, state.elapsed, timeline, melee, swing,
      cfg.attackEffect.fadeSec, ReactionLevel::Blow, state.hitboxEntity,
      offsetFn
  );
  UpdateAttackLights(
      registry, entity, state.elapsed, timeline,
      LightSpec{
          .count = finisher.lightCount,
          .radius = swing.radius,
          .fadeSec = cfg.attackEffect.fadeSec
      },
      state.lightEntities,
      [&offsetFn, &finisher](double progress, int32 index) {
        Vec3 offset = offsetFn(progress);
        offset.y += MeleeLightSpread(progress, finisher, index);
        return offset;
      }
  );

  // 締め段はコンボ継続・キャンセルを受け付けないため、満了のみで遷移する
  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
