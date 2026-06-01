#include "System/PlayerMovementSystem.hpp"

#include "Component/Player.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

void PlayerMovementSystem::Update(entt::registry& registry,
                                  const FrameData& frameData,
                                  const s3d::String& attackClip) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;
  const double dt = frameData.dt;
  const bool isAttacking = !attackClip.empty();

  auto view = registry.view<Player, WorldPos, Velocity, SpriteAnimation>();
  for (auto&& [entity, pos, vel, anim] : view.each()) {
    if (isAttacking) {
      vel.w = 0.0;
      vel.d = 0.0;
    } else {
      vel.w = input.moveRight ? cfg.speed : input.moveLeft ? -cfg.speed : 0.0;
      vel.d = input.moveForward    ? cfg.speed
              : input.moveBackward ? -cfg.speed
                                   : 0.0;
    }

    pos.w += vel.w * dt;
    pos.d += vel.d * dt;

    vel.h -= cfg.gravity * dt;
    pos.h += vel.h * dt;

    if (pos.h < 0.0) {
      pos.h = 0.0;
      vel.h = 0.0;
    }

    const bool onGround = pos.isOnGround();

    if (input.jumpDown && onGround) {
      vel.h = cfg.jumpSpeed;
    }

    // アニメーションクリップ決定（攻撃 > ジャンプ > 移動 > 待機）
    const s3d::String newClip = isAttacking                      ? attackClip
                                : (pos.h > 0.0)                  ? U"jump"
                                : (vel.w != 0.0 || vel.d != 0.0) ? U"move"
                                                                 : U"idle";
    if (newClip != anim.currentClip) {
      anim.currentClip = newClip;
      anim.elapsed = 0.0;
    }

    // 向き更新（攻撃中は変更しない）
    if (!isAttacking) {
      if (vel.w > 0.0) {
        anim.facingRight = true;
      } else if (vel.w < 0.0) {
        anim.facingRight = false;
      }
    }
  }
}
