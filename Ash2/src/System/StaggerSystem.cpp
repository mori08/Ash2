#include <Siv3D.hpp>

#include "System/StaggerSystem.hpp"

#include "Component/Drawable.hpp"
#include "Component/Stagger.hpp"

namespace {

/// @brief ひるみ表現の最大縮小率（縦方向）
constexpr double KMaxShrinkRatio = 0.2;

}  // namespace

void StaggerSystem::Update(entt::registry& registry, double dt) {
  auto view = registry.view<Stagger, Drawable>();
  for (auto&& [entity, stagger, drawable] : view.each()) {
    stagger.remaining -= dt;

    auto* rect = std::get_if<RectDrawable>(&drawable);
    if (rect == nullptr) continue;

    if (stagger.remaining <= 0.0) {
      rect->size = stagger.originalSize;
      continue;
    }

    // duration の中間で最も縮み、両端（開始・終了）で原寸に近づく
    const double progress = stagger.remaining / stagger.duration;
    const double shrink = (1.0 - Abs(progress * 2.0 - 1.0)) * KMaxShrinkRatio;
    rect->size.y = stagger.originalSize.y * (1.0 - shrink);
  }

  // remaining <= 0 になったエンティティから Stagger を除去する
  Array<entt::entity> expired;
  for (const auto entity : view) {
    if (view.get<Stagger>(entity).remaining <= 0.0) {
      expired.push_back(entity);
    }
  }
  for (const auto entity : expired) {
    registry.remove<Stagger>(entity);
  }
}
