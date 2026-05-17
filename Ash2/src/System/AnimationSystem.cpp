#include "System/AnimationSystem.hpp"

#include "Component/AnimationData.hpp"
#include "Component/Drawable.hpp"
#include "Component/SpriteAnimation.hpp"

void AnimationSystem::Update(entt::registry& registry, double dt) {
  const auto& dataRegistry = registry.ctx().get<AnimationDataRegistry>();

  auto view = registry.view<SpriteAnimation, Drawable>();
  for (auto [entity, anim, drawable] : view.each()) {
    const auto& data = dataRegistry.at(anim.dataKey);
    const auto& clip = data.clips.at(anim.currentClip);

    anim.elapsed += dt;

    const int col = static_cast<int>(anim.elapsed * clip.speed) % clip.count;
    auto region = TextureAsset{data.textureKey}(
        col * data.size.x, clip.row * data.size.y, data.size.x, data.size.y);

    if (anim.facingRight) {
      region = region.mirrored();
    }

    if (auto* td = std::get_if<TextureDrawable>(&drawable)) {
      td->region = region;
      td->drawOffset = data.drawOffset;
    }
  }
}
