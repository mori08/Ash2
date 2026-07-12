#include <Siv3D.hpp>

#include "System/PlayerMotionSystem.hpp"

#include <functional>

#include "Component/Attack.hpp"
#include "Component/Collider.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/Invincible.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/Projectile.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Stamina.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/AnimationData.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

namespace PlayerMotion {

namespace {

constexpr ColorF KBulletColor = {0.9, 0.9, 0.3};
constexpr ColorF KMeleeOrbColor = {1.0, 0.9, 0.5};
/// 暫定のヒットストップ時間（秒）。本格的な数値調整は #132/#134 で行う
constexpr double KMeleeHitstopSec = 0.05;

/// @brief 指定クリップの再生時間（秒）を返す
/// @return クリップが見つからない場合は 0.0
double GetClipDuration(const AnimationData& data, const String& clip) {
  const auto it = data.clips.find(clip);
  if (it == data.clips.end()) return 0.0;
  return static_cast<double>(it->second.count) / it->second.speed;
}

/// @brief 横方向の速度を止める
void StopHorizontalMovement(entt::registry& registry, entt::entity entity) {
  auto& vel = registry.get<Velocity>(entity);
  vel.w = 0.0;
  vel.d = 0.0;
}

/// @brief クリップが変化していれば差し替え、再生位置をリセットする
void SetClip(SpriteAnimation& anim, const String& clip) {
  if (clip != anim.currentClip) {
    anim.currentClip = clip;
    anim.elapsed = 0.0;
  }
}

/// @brief クリップ名が同じでも再生位置を先頭へ強制的に戻す
void RestartClip(SpriteAnimation& anim, const String& clip) {
  anim.currentClip = clip;
  anim.elapsed = 0.0;
}

/// @brief 攻撃判定エンティティの半径・ダメージ量をまとめた仕様
// SpawnAttackHitbox の引数で double の radius と int の damage が隣接すると
// 呼び出し側で取り違えやすいため、1つの構造体にまとめて渡す
// （bugprone-easily-swappable-parameters 対策）。
struct HitboxSpec {
  /// 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
  double radius = 0.0;
  /// 与えるダメージ量
  int damage = 0;
};

/// @brief 攻撃判定エンティティ（光の珠）を生成する
///
/// Component/Attack.hpp の Attack（ダメージ用）を spec.damage
/// で確定させて付与する（生成後の上書きは行わない）。生成時点では構え中につき
/// 珠は体の近くに静止した位置に置く。Collider は珠エンティティ自身の原点からの
/// オフセット 0 で固定し、珠の現在位置は UpdateAttackHitbox が更新する
/// LocalOffset のみが担う。
entt::entity SpawnAttackHitbox(entt::registry& registry, entt::entity owner,
                               const WorldPos& pos, const HitboxSpec& spec) {
  const auto hitbox = registry.create();
  registry.emplace<WorldPos>(hitbox, pos);
  registry.emplace<LocalOffset>(hitbox, LocalOffset{});
  Hierarchy::Attach(registry, owner, hitbox);
  registry.emplace<Collider>(hitbox, Collider{.segmentStart = Vec3::Zero(),
                                              .segmentEnd = Vec3::Zero(),
                                              .radius = spec.radius});
  registry.emplace<Attack>(
      hitbox, Attack{.damage = spec.damage, .hitstopSec = KMeleeHitstopSec});
  registry.emplace<Drawable>(
      hitbox, CircleDrawable{.radius = spec.radius, .color = KMeleeOrbColor});
  return hitbox;
}

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

/// @brief 攻撃判定の発生区間に応じてヒットボックスを生成・更新・破棄する
/// @param timeline 攻撃のタイムライン（active 区間の判定に使用）
/// @param radius 攻撃カプセルの半径（兼 CircleDrawable の表示半径）
/// @param damage 生成時に確定させるダメージ量
/// @param offsetFn 攻撃フレーム内の進行度から珠のオフセットを算出する関数
void UpdateAttackHitbox(entt::registry& registry, entt::entity owner,
                        double elapsed, const MotionTimeline& timeline,
                        double radius, int damage, entt::entity& hitboxEntity,
                        const std::function<Vec3(double)>& offsetFn) {
  // 攻撃フレーム中（未生成ならここで生成する）：珠を前方へ EaseOut
  // 補間で移動させる
  if (timeline.isActive(elapsed)) {
    if (hitboxEntity == entt::null) {
      const auto& pos = registry.get<WorldPos>(owner);
      hitboxEntity = SpawnAttackHitbox(
          registry, owner, pos, HitboxSpec{.radius = radius, .damage = damage});
    }

    const auto offset = offsetFn(timeline.activeProgress(elapsed));

    auto& localOffset = registry.get<LocalOffset>(hitboxEntity);
    localOffset.value = WorldPos{.w = offset.x, .h = offset.y, .d = offset.z};
  }

  // 後隙以降：ヒットボックスが残っていれば破棄する
  if (elapsed >= timeline.activeEnd() && hitboxEntity != entt::null) {
    Hierarchy::DestroyWithChildren(registry, hitboxEntity);
    hitboxEntity = entt::null;
  }
}

/// @brief 遠距離攻撃の弾エンティティを生成する
void SpawnProjectile(entt::registry& registry, const WorldPos& pos,
                     bool facingRight, const PlayerConfig& cfg) {
  const double sign = facingRight ? 1.0 : -1.0;
  const auto bullet = registry.create();
  registry.emplace<WorldPos>(
      bullet,
      WorldPos{.w = pos.w, .h = pos.h + cfg.ranged.spawnHeight, .d = pos.d});
  registry.emplace<Velocity>(bullet,
                             Velocity{.w = sign * cfg.ranged.bulletSpeed});
  registry.emplace<Collider>(bullet, Collider{
                                         .segmentStart = Vec3{0.0, 0.0, 0.0},
                                         .segmentEnd = Vec3{0.0, 0.0, 0.0},
                                         .radius = cfg.ranged.radius,
                                     });
  registry.emplace<Attack>(bullet, Attack{.damage = cfg.ranged.damage});
  registry.emplace<Drawable>(bullet, CircleDrawable{.radius = cfg.ranged.radius,
                                                    .color = KBulletColor});
  registry.emplace<Projectile>(bullet);
}

/// @brief 指定段の Melee へ移行する（攻撃クリップを先頭から再生）
/// @param stage 移行先のコンボ段インデックス
/// @param hitboxEntity 引き継ぐ攻撃判定エンティティ（通常は entt::null）
Melee MakeMelee(SpriteAnimation& anim, int stage,
                entt::entity hitboxEntity = entt::null) {
  // Neutral からの初回入場でもクリップは melee_1 以外から必ず切り替わるため、
  // RestartClip と SetClip の挙動差は生じない。
  RestartClip(anim, U"melee_1");
  return Melee{.stage = stage, .elapsed = 0.0, .hitboxEntity = hitboxEntity};
}

/// @brief Ranged へ移行する（スタミナ消費、遠距離攻撃クリップの設定と timer
/// の算出）
Ranged MakeRanged(entt::registry& registry, entt::entity entity,
                  const PlayerConfig& cfg, const AnimationData& playerData,
                  SpriteAnimation& anim) {
  auto& stamina = registry.get<Stamina>(entity);
  stamina.current = Max(0, stamina.current - cfg.ranged.staminaCost);
  SetClip(anim, U"ranged_attack");
  return Ranged{.timer = GetClipDuration(playerData, U"ranged_attack")};
}

/// @brief Dash へ移行する（スタミナ消費、クリップの設定）
///
/// 専用のダッシュ用クリップは未用意のため、移動主体の動きという特性が近い
/// "move" クリップを暫定的に流用する。
/// @param air 空中発動か（true: 空中ダッシュ相当）
Dash MakeDash(entt::registry& registry, entt::entity entity,
              const PlayerConfig& cfg, SpriteAnimation& anim, bool air) {
  auto& stamina = registry.get<Stamina>(entity);
  stamina.current = Max(0, stamina.current - cfg.dash.staminaCost);
  SetClip(anim, U"move");
  return Dash{.air = air};
}

/// @brief
/// 攻撃フレーム内の進行度からダッシュ攻撃の珠オフセット（水平軌道）を返す
///
/// Vec3 の x が w 軸（横）、z が d 軸（奥行き）、y が高さ固定（capMidH）。
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
// progress / orbitRadius は隣接する double 引数として渡すと取り違えやすいため
// （bugprone-easily-swappable-parameters 対策）、orbitRadius は
// DashAttackConfig への const 参照経由で受け取る。
Vec3 DashAttackOrbOffset(double progress, const DashAttackConfig& da,
                         double capMidH) {
  const double angle = Math::TwoPi * progress;
  return Vec3{da.orbitRadius * Math::Cos(angle), capMidH,
              da.orbitRadius * Math::Sin(angle)};
}

/// @brief DashAttack へ移行する
/// @param air 空中発動か（true: 空中ダッシュ攻撃相当）
/// @param dashDir ダッシュ時の移動方向（正規化済み）
DashAttack MakeDashAttack(SpriteAnimation& anim, bool air, Vec2 dashDir) {
  // 専用クリップ未用意のため "melee_1" を暫定流用する
  SetClip(anim, U"melee_1");
  return DashAttack{.elapsed = 0.0,
                    .air = air,
                    .hitboxEntity = entt::null,
                    .dashDir = dashDir};
}

/// @brief 攻撃フレーム内の進行度から空中攻撃の珠オフセット（垂直軌道）を返す
///
/// Vec3 の x が w 軸（横）、y が高さ（capMidH を中心に周回）、z は 0
/// 固定。DashAttack の w-d 平面軌道に対し、こちらは w-h 平面（垂直面）を
/// 周回する。回転方向はプレイヤーの向きに応じて左右反転する。
/// @param progress 攻撃フレーム内の進行度（0.0〜1.0）
/// @param facingRight プレイヤーの向き（false なら w 成分の符号を反転）
// progress / orbitRadius は隣接する double 引数として渡すと取り違えやすいため
// （bugprone-easily-swappable-parameters 対策）、orbitRadius は
// AirAttackConfig への const 参照経由で受け取る。
Vec3 AirAttackOrbOffset(double progress, const AirAttackConfig& aa,
                        double capMidH, bool facingRight) {
  const double angle = Math::TwoPi * progress;
  const double w = facingRight ? aa.orbitRadius * Math::Cos(angle)
                               : -aa.orbitRadius * Math::Cos(angle);
  return Vec3{w, capMidH - aa.orbitRadius * Math::Sin(angle), 0.0};
}

/// @brief AirAttack へ移行する
AirAttack MakeAirAttack(SpriteAnimation& anim) {
  // 専用クリップ未用意のため "melee_1" を暫定流用する
  SetClip(anim, U"melee_1");
  return AirAttack{.elapsed = 0.0, .hitboxEntity = entt::null};
}

}  // namespace

Optional<Motion> Tick(Neutral& /*state*/, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  const auto& input = frameData.input;
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& playerData =
      registry.ctx().get<AnimationDataRegistry>().at(U"player");
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  // 移動・ジャンプ・向き
  vel.w = input.moveAxis.x * cfg.speed;
  vel.d = input.moveAxis.y * cfg.speed;

  if (vel.w > 0.0) {
    anim.facingRight = true;
  } else if (vel.w < 0.0) {
    anim.facingRight = false;
  }

  // Melee/Ranged への入場（接地中のみ）
  if (pos.isOnGround()) {
    if (input.attackDown) {
      // 直前で設定した横方向速度を打ち消す（持ち越すと1フレーム分滑る）
      vel.w = 0.0;
      vel.d = 0.0;
      // ヒットボックス（光の珠）は攻撃フレーム開始時に Melee::Tick が生成する
      return MakeMelee(anim, 0);
    }
    if (input.rangedAttackDown &&
        registry.get<Stamina>(entity).current >= cfg.ranged.staminaCost) {
      vel.w = 0.0;
      vel.d = 0.0;
      SpawnProjectile(registry, pos, anim.facingRight, cfg);
      return MakeRanged(registry, entity, cfg, playerData, anim);
    }
    if (input.dashDown &&
        registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
      return MakeDash(registry, entity, cfg, anim, /*air=*/false);
    }
  } else if (input.attackDown) {
    // 空中攻撃への入場（AirAttack::Tick が接地検出で Landing へ遷移させる）
    vel.w = 0.0;
    vel.d = 0.0;
    return MakeAirAttack(anim);
  } else if (input.rangedAttackDown &&
             registry.get<Stamina>(entity).current >= cfg.ranged.staminaCost) {
    // 空中遠距離攻撃への入場。Ranged は地上・空中で共有するため、
    // 着地しても Landing を挟まずタイマー満了で Neutral
    // に戻る（地上と同一挙動）。
    vel.w = 0.0;
    vel.d = 0.0;
    SpawnProjectile(registry, pos, anim.facingRight, cfg);
    return MakeRanged(registry, entity, cfg, playerData, anim);
  } else if (input.dashDown &&
             registry.get<Stamina>(entity).current >= cfg.dash.staminaCost) {
    // 空中ダッシュへの入場（Dash::Tick が air フラグにより接地検出で Landing
    // へ遷移させる）
    return MakeDash(registry, entity, cfg, anim, /*air=*/true);
  }

  if (input.jumpDown && pos.isOnGround()) {
    vel.h = cfg.jumpSpeed;
  }

  // ロコモーションクリップ（idle/move/jump）
  // pos は前フレームまでの値のため、ジャンプ入力で vel.h を設定した
  // 直後のフレームでは isOnGround() がまだ true のままになる。
  // vel.h > 0.0（ジャンプによる上昇）の場合のみ判定に加えることで、
  // 重力による接地中の微小な負の vel.h には反応せず、
  // 入力と同フレームで jump クリップに切り替える。
  if (!pos.isOnGround() || vel.h > 0.0) {
    SetClip(anim, U"jump");
  } else if (vel.w != 0.0 || vel.d != 0.0) {
    SetClip(anim, U"move");
  } else {
    SetClip(anim, U"idle");
  }

  return none;
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

  const auto offsetFn = MakeMeleeOffsetFn(stageCfg, anim.facingRight, melee);
  UpdateAttackHitbox(registry, entity, state.elapsed, timeline, stageCfg.radius,
                     melee.damage, state.hitboxEntity, offsetFn);

  const bool hasNextStage =
      state.stage + 1 < static_cast<int>(melee.stages.size());

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

Optional<Motion> Tick(Ranged& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }

  return none;
}

Optional<Motion> Tick(Dash& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
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

Optional<Motion> Tick(DashAttack& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& da = cfg.dashAttack;
  const auto& timeline = da.timeline;
  const auto& pos = registry.get<WorldPos>(entity);
  auto& vel = registry.get<Velocity>(entity);
  auto& anim = registry.get<SpriteAnimation>(entity);

  state.elapsed += frameData.dt;

  // 接地検出は後隙中も含め毎フレーム優先して評価する（タイマー満了判定より先、
  // 空中発動時のみ。地上 DashAttack は接地遷移を持たない）
  if (state.air && pos.isOnGround()) {
    if (state.hitboxEntity != entt::null) {
      Hierarchy::DestroyWithChildren(registry, state.hitboxEntity);
      state.hitboxEntity = entt::null;
    }
    return Landing{.timer = cfg.landing.recoverySec};
  }

  // 攻撃判定区間：ヒットボックスの生成・軌道更新・後隙以降の破棄
  const double capMidH = cfg.melee.capMidH;
  const auto offsetFn = [&da, capMidH](double progress) {
    return DashAttackOrbOffset(progress, da, capMidH);
  };
  UpdateAttackHitbox(registry, entity, state.elapsed, timeline, da.radius,
                     da.damage, state.hitboxEntity, offsetFn);

  // 突進フェーズ（構え）：ダッシュ方向へ移動
  // 空中発動時は AirDash の暫定仕様に合わせ垂直速度を 0 に固定する
  if (state.elapsed < timeline.activeStart()) {
    vel.w = state.dashDir.x * da.speed;
    vel.d = state.dashDir.y * da.speed;
    if (state.air) vel.h = 0.0;
  } else {
    vel.w = 0.0;
    vel.d = 0.0;
  }

  if (state.dashDir.x > 0.0) {
    anim.facingRight = true;
  } else if (state.dashDir.x < 0.0) {
    anim.facingRight = false;
  }

  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

Optional<Motion> Tick(AirAttack& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& aa = cfg.airAttack;
  const auto& timeline = aa.timeline;
  const auto& pos = registry.get<WorldPos>(entity);

  state.elapsed += frameData.dt;

  // 接地検出は後隙中も含め毎フレーム優先して評価する（タイマー満了判定より先）
  if (pos.isOnGround()) {
    if (state.hitboxEntity != entt::null) {
      Hierarchy::DestroyWithChildren(registry, state.hitboxEntity);
      state.hitboxEntity = entt::null;
    }
    return Landing{.timer = cfg.landing.recoverySec};
  }

  const auto& anim = registry.get<SpriteAnimation>(entity);
  const bool facingRight = anim.facingRight;
  const auto offsetFn = [&aa, &cfg, facingRight](double progress) {
    return AirAttackOrbOffset(progress, aa, cfg.melee.capMidH, facingRight);
  };
  UpdateAttackHitbox(registry, entity, state.elapsed, timeline, aa.radius,
                     aa.damage, state.hitboxEntity, offsetFn);

  // 接地せずに終わった場合はタイマー満了で Neutral へ戻る
  if (timeline.isFinished(state.elapsed)) {
    return Neutral{};
  }

  return none;
}

Optional<Motion> Tick(Landing& state, entt::registry& registry,
                      entt::entity entity, const FrameData& frameData) {
  StopHorizontalMovement(registry, entity);

  // 専用クリップ未用意のため "idle" を暫定流用
  auto& anim = registry.get<SpriteAnimation>(entity);
  SetClip(anim, U"idle");

  state.timer -= frameData.dt;
  if (state.timer <= 0.0) {
    return Neutral{};
  }

  return none;
}

}  // namespace PlayerMotion
