#include "System/GravitySystem.hpp"

#include "Component/Gravity.hpp"
#include "Component/Velocity.hpp"
#include "Component/WorldPos.hpp"

void GravitySystem::Update(entt::registry& registry, double dt) {
  auto view = registry.view<WorldPos, Velocity, Gravity>();
  for (auto&& [entity, pos, vel, gravity] : view.each()) {
    // 次フレーム用の重力加速
    vel.h -= gravity.accel * dt;

    // 今フレームの位置に対する地面クランプ
    if (pos.h < 0.0) {
      pos.h = 0.0;
      vel.h = 0.0;
    }
  }
}
