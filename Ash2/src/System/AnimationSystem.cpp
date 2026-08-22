#include "System/AnimationSystem.hpp"

#include <cassert>

#include "Component/Drawable.hpp"
#include "Component/Hitstop.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/AnimationData.hpp"

void AnimationSystem::Update(entt::registry& registry, double dt) {
  const auto& dataRegistry = registry.ctx().get<AnimationDataRegistry>();

  auto view = registry.view<SpriteAnimation, Drawable>(entt::exclude<Hitstop>);
  for (auto [entity, anim, drawable] : view.each()) {
    assert(
        dataRegistry.contains(anim.dataKey) &&
        "AnimationDataRegistry にキーが存在しない"
    );
    const auto& data = dataRegistry.at(anim.dataKey);
    assert(
        data.clips.contains(anim.currentClip) && "clips にクリップが存在しない"
    );
    const auto& clip = data.clips.at(anim.currentClip);

    anim.elapsed += dt;

    assert(clip.count > 0 && "clip.count は正の値でなければならない");
    assert(clip.speed > 0.0 && "clip.speed は正の値でなければならない");
    const double cycleDuration = clip.count / clip.speed;
    anim.elapsed = Math::Fmod(anim.elapsed, cycleDuration);
    const int32 col =
        static_cast<int32>(anim.elapsed * clip.speed) % clip.count;
    const Point cell{col, clip.row};
    // data.textureKey の登録・読み込み済みは LoadAnimations（GameSetup.cpp）
    // がロード時に保証する
    assert(
        TextureAsset::IsRegistered(data.textureKey) &&
        "TextureAsset にキーが登録されていない"
    );
    auto region = TextureAsset{data.textureKey}(cell * data.size, data.size);

    if (anim.facingRight) {
      region = region.mirrored();
    }

    if (auto* td = std::get_if<TextureDrawable>(&drawable)) {
      td->region = region;
      td->drawOffset = data.drawOffset;
    }
  }
}
