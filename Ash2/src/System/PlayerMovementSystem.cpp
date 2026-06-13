#include "System/PlayerMovementSystem.hpp"

#include "Component/AttackState.hpp"
#include "Component/NeutralState.hpp"
#include "Component/Player.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"
#include "Config/PlayerConfig.hpp"
#include "Phase/FrameData.hpp"

void PlayerMovementSystem::Update(entt::registry& registry,
                                  const FrameData& frameData) {
  const auto& cfg = registry.ctx().get<PlayerConfig>();
  const auto& input = frameData.input;

  auto view = registry.view<Player, WorldPos, Velocity, SpriteAnimation>();
  for (auto&& [entity, pos, vel, anim] : view.each()) {
    const bool canMove = registry.all_of<NeutralState>(entity);

    if (!canMove) {
      vel.w = 0.0;
      vel.d = 0.0;
    } else {
      vel.w = input.moveAxis.x * cfg.speed;
      vel.d = input.moveAxis.y * cfg.speed;
    }

    const bool onGround = pos.isOnGround();

    if (input.jumpDown && onGround) {
      vel.h = cfg.jumpSpeed;
    }

    // アニメーションクリップ決定（攻撃 > ジャンプ > 移動 > 待機）
    s3d::String newClip;
    if (!canMove) {
      const auto* attackState = registry.try_get<AttackState>(entity);
      newClip = (attackState != nullptr) ? attackState->clip : s3d::String{};
    } else if (pos.h > 0.0) {
      newClip = U"jump";
    } else if (vel.w != 0.0 || vel.d != 0.0) {
      newClip = U"move";
    } else {
      newClip = U"idle";
    }
    if (newClip != anim.currentClip) {
      anim.currentClip = newClip;
      anim.elapsed = 0.0;
    }

    // 向き更新（攻撃中は変更しない）
    if (canMove) {
      if (vel.w > 0.0) {
        anim.facingRight = true;
      } else if (vel.w < 0.0) {
        anim.facingRight = false;
      }
    }
  }
}
