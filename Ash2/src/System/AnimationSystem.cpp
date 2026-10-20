#include "System/AnimationSystem.hpp"

#include "Component/Drawable.hpp"
#include "Component/Hitstop.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/AnimationData.hpp"

void AnimationSystem::Update(entt::registry& registry, double dt) {
  const auto& dataRegistry = registry.ctx().get<AnimationDataRegistry>();

  auto view = registry.view<SpriteAnimation, Drawable>(entt::exclude<Hitstop>);
  for (auto [entity, anim, drawable] : view.each()) {
    if (!dataRegistry.contains(anim.dataKey)) {
      throw Error{U"AnimationDataRegistry にデータキーがない"};
    }
    const auto& data = dataRegistry.at(anim.dataKey);
    if (!data.clips.contains(anim.currentClip)) {
      throw Error{U"アニメーションデータにクリップがない"};
    }
    const auto& clip = data.clips.at(anim.currentClip);

    if (clip.count <= 0) {
      throw Error{U"クリップのコマ数が正の値でない"};
    }
    if (clip.speed <= 0.0) {
      throw Error{U"クリップの速度が正の値でない"};
    }
    anim.elapsed = clip.advance(anim.elapsed, dt);
    const int32 col = clip.columnAt(anim.elapsed);
    const Point cell{col, clip.row};
    // data.textureKey の登録・読み込み済みは LoadAnimations（GameSetup.cpp）
    // がロード時に保証する
    if (!TextureAsset::IsRegistered(data.textureKey)) {
      throw Error{U"TextureAsset にテクスチャのキーが登録されていない"};
    }
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
