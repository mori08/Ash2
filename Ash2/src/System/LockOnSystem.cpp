#include "System/LockOnSystem.hpp"

#include "Component/Collider.hpp"
#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/Enemy.hpp"
#include "Component/EnemyMotion.hpp"
#include "Component/Hierarchy.hpp"
#include "Component/LocalOffset.hpp"
#include "Component/LockOn.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/WorldPos.hpp"
#include "FrameData.hpp"

namespace {

// aim point の d は Collider のオフセット次第で target 自身の WorldPos.d と
// 一致しうる（例: オフセットが0）。その場合の描画順の同値を避けるため、
// レティクルの d をわずかに手前へ寄せる
constexpr double kReticleDrawOrderEpsilon = 0.01;

/// @brief 敵として有効なロック対象か（生存し、Defeated でない）
[[nodiscard]] bool IsAliveEnemy(
    const entt::registry& registry, entt::entity e
) {
  if (e == entt::null || !registry.valid(e)) return false;
  const auto* motion = registry.try_get<EnemyMotion::Variant>(e);
  if (motion == nullptr) return false;
  return !std::holds_alternative<EnemyMotion::Defeated>(*motion);
}

/// @brief マウス規則でロック対象を決める
///
/// カーソルを含む敵のうち、`|enemy.d - player.d|` が最小のものを target
/// にする（含む敵がなければ解除）。halfTarget と stickTilted は常に
/// 初期状態へ戻す。
void ApplyMouseRule(
    const entt::registry& registry, const WorldPos& playerPos, Vec2 pointerPos,
    const LockConfig& cfg, LockOn& lockOn
) {
  // カメラオフセットを1回だけ差し引き、以降は Project() の投影と同じ
  // 「カメラオフセットを含まない画面座標」で比較する
  const Vec2 cursor = pointerPos - Scene::Center();

  entt::entity best = entt::null;
  double bestDepthDiff = Math::Inf;

  auto view = registry.view<
      const Enemy, const WorldPos, const Collider,
      const EnemyMotion::Variant>();
  for (const auto entity : view) {
    if (std::holds_alternative<EnemyMotion::Defeated>(
            view.get<const EnemyMotion::Variant>(entity)
        )) {
      continue;
    }

    const auto& pos = view.get<const WorldPos>(entity);
    const auto& col = view.get<const Collider>(entity);
    const auto cap = LockOnSystem::Project(pos, col, cfg.capsuleScale);
    if (!LockOnSystem::Contains(cap, cursor)) continue;

    const double depthDiff = Abs(pos.d - playerPos.d);
    if (depthDiff < bestDepthDiff) {
      bestDepthDiff = depthDiff;
      best = entity;
    }
  }

  lockOn.target = best;
  // マウス規則に半ロックの段階はない。スティックから切り替わった直後に
  // 古い halfTarget と stickTilted が残らないよう、毎フレーム戻す
  lockOn.halfTarget = entt::null;
  lockOn.stickTilted = false;
}

/// @brief スティック規則でロック対象・半ロック対象を決める
///
/// 傾き 0.5 / 離し 0.24 の2段階でヒステリシスを取る。
void ApplyStickRule(
    const entt::registry& registry, entt::entity player, Vec2 lockAxis,
    const LockConfig& cfg, LockOn& lockOn
) {
  const double magnitude = lockAxis.length();

  if (!lockOn.stickTilted && magnitude >= cfg.stickTilt) {
    lockOn.stickTilted = true;
  } else if (lockOn.stickTilted && magnitude <= cfg.stickRelease) {
    lockOn.stickTilted = false;
    // 離した瞬間に SelectByDirection をやり直さない。中立に戻った lockAxis
    // からは倒していた向きが読めず、意図しない敵に確定してしまう
    lockOn.target = lockOn.halfTarget;
    lockOn.halfTarget = entt::null;
  }

  if (lockOn.stickTilted) {
    const entt::entity axis =
        (lockOn.target != entt::null) ? lockOn.target : player;
    lockOn.halfTarget =
        LockOnSystem::SelectByDirection(registry, axis, lockAxis, cfg);
  }
}

/// @brief target に新しいレティクルを生成し、狙点へアタッチする
[[nodiscard]] entt::entity SpawnReticle(
    entt::registry& registry, entt::entity target
) {
  const auto& pos = registry.get<WorldPos>(target);
  const auto& col = registry.get<Collider>(target);
  const WorldPos aim = LockOnSystem::AimPoint(pos, col);

  const auto reticle = registry.create();
  // AttachmentSystem が上書きするまでの初期値として親の現在位置を置く
  registry.emplace<WorldPos>(reticle, pos);
  // region は空のまま。同フレーム内で AnimationSystem が埋める
  registry.emplace<Drawable>(reticle, TextureDrawable{});
  registry.emplace<SpriteAnimation>(
      reticle, SpriteAnimation{.dataKey = U"reticle", .currentClip = U"idle"}
  );
  registry.emplace<DrawColor>(reticle);
  Hierarchy::Attach(
      registry, target, reticle,
      LocalOffset{
          .w = aim.w - pos.w,
          .h = aim.h - pos.h,
          .d = aim.d - pos.d - kReticleDrawOrderEpsilon,
      }
  );
  return reticle;
}

/// @brief target に追従するレティクルを同期する
///
/// target がなければ破棄する。親が変わっていれば作り直し（Hierarchy::parent()
/// で判定）、そうでなければ色だけを更新する。
void SyncReticle(
    entt::registry& registry, entt::entity target, entt::entity& reticle,
    const ColorF& color
) {
  if (target == entt::null) {
    if (reticle != entt::null && registry.valid(reticle)) {
      Hierarchy::DestroyWithChildren(registry, reticle);
    }
    reticle = entt::null;
    return;
  }

  const bool needsRespawn =
      reticle == entt::null || !registry.valid(reticle) ||
      registry.get<Hierarchy>(reticle).parent() != target;
  if (needsRespawn) {
    if (reticle != entt::null && registry.valid(reticle)) {
      Hierarchy::DestroyWithChildren(registry, reticle);
    }
    reticle = SpawnReticle(registry, target);
  }

  registry.get<DrawColor>(reticle).color = color;
}

}  // namespace

WorldPos LockOnSystem::AimPoint(const WorldPos& pos, const Collider& col) {
  const Vec3 mid = (col.segmentStart + col.segmentEnd) / 2.0;
  return WorldPos{.w = pos.w + mid.x, .h = pos.h + mid.y, .d = pos.d + mid.z};
}

ScreenCapsule LockOnSystem::Project(
    const WorldPos& pos, const Collider& col, double scale
) {
  const WorldPos start{
      .w = pos.w + col.segmentStart.x,
      .h = pos.h + col.segmentStart.y,
      .d = pos.d + col.segmentStart.z,
  };
  const WorldPos end{
      .w = pos.w + col.segmentEnd.x,
      .h = pos.h + col.segmentEnd.y,
      .d = pos.d + col.segmentEnd.z,
  };
  return ScreenCapsule{
      .start = start.toScreen(),
      .end = end.toScreen(),
      .radius = col.radius * scale,
  };
}

bool LockOnSystem::Contains(const ScreenCapsule& cap, Vec2 point) {
  const Vec2 segment = cap.end - cap.start;
  const double lengthSq = segment.lengthSq();
  // 線分が退化して点になる形状（Collider のオフセットが0のケース）を
  // 割る前にガードする
  const double t =
      (lengthSq > 0.0)
          ? Clamp((point - cap.start).dot(segment) / lengthSq, 0.0, 1.0)
          : 0.0;
  const Vec2 closest = cap.start + segment * t;
  return closest.distanceFrom(point) <= cap.radius;
}

entt::entity LockOnSystem::SelectByDirection(
    const entt::registry& registry, entt::entity axis, Vec2 dir,
    const LockConfig& cfg
) {
  if (dir.isZero()) return entt::null;

  const Vec2 axisScreen =
      AimPoint(registry.get<WorldPos>(axis), registry.get<Collider>(axis))
          .toScreen();

  entt::entity best = entt::null;
  double bestScore = Math::Inf;

  auto view = registry.view<
      const Enemy, const WorldPos, const Collider,
      const EnemyMotion::Variant>();
  for (const auto entity : view) {
    if (entity == axis) continue;
    if (std::holds_alternative<EnemyMotion::Defeated>(
            view.get<const EnemyMotion::Variant>(entity)
        )) {
      continue;
    }

    const Vec2 candidateScreen =
        AimPoint(
            view.get<const WorldPos>(entity), view.get<const Collider>(entity)
        )
            .toScreen();
    const Vec2 toCandidate = candidateScreen - axisScreen;
    if (toCandidate.isZero()) continue;

    const double angleDeg = Abs(Math::ToDegrees(dir.getAngle(toCandidate)));
    if (angleDeg > cfg.angleLimitDeg) continue;

    const double score =
        toCandidate.length() *
        (1.0 + cfg.angleWeight * angleDeg / cfg.angleLimitDeg);
    if (score < bestScore) {
      bestScore = score;
      best = entity;
    }
  }

  return best;
}

void LockOnSystem::Update(
    entt::registry& registry, const FrameData& frameData
) {
  const auto& input = frameData.input;
  const auto& cfg = registry.ctx().get<PlayerConfig>().lock;

  // LockOn は入場時にプレイヤーへ1つだけ付与される想定
  //
  // WorldPos はこのループ内でレティクルの生成・破棄により増減するため、
  // ビューには含めない（ビュー走査中に自分が束ねるプールを変更しない）。
  // playerPos は player の WorldPos を個別に取得する
  auto view = registry.view<LockOn>();
  for (auto&& [player, lockOn] : view.each()) {
    const auto& playerPos = registry.get<const WorldPos>(player);
    if (!IsAliveEnemy(registry, lockOn.target)) lockOn.target = entt::null;
    if (!IsAliveEnemy(registry, lockOn.halfTarget)) {
      lockOn.halfTarget = entt::null;
    }

    if (input.pointerPos) {
      ApplyMouseRule(registry, playerPos, *input.pointerPos, cfg, lockOn);
    } else {
      ApplyStickRule(registry, player, input.lockAxis, cfg, lockOn);
    }

    // 解除予告: スティックを倒しているのに乗り換え先の候補がない状態
    const bool isWarning =
        lockOn.stickTilted && lockOn.halfTarget == entt::null;
    const ColorF targetColor = cfg.lockedColor.withAlpha(
        isWarning ? cfg.warningAlpha : cfg.lockedAlpha
    );
    SyncReticle(registry, lockOn.target, lockOn.targetReticle, targetColor);

    const ColorF halfColor = cfg.halfColor.withAlpha(cfg.halfAlpha);
    SyncReticle(registry, lockOn.halfTarget, lockOn.halfReticle, halfColor);
  }
}
