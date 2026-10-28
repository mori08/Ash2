#include "System/AnimationSystem.hpp"

#include "Component/Drawable.hpp"
#include "Component/Hitstop.hpp"
#include "Component/SpriteAnimation.hpp"
#include "Config/AnimationData.hpp"

void AnimationSystem::Update(entt::registry& registry, double dt) {
  const auto& dataRegistry = registry.ctx().get<AnimationDataRegistry>();

  auto view = registry.view<SpriteAnimation, Drawable>(entt::exclude<Hitstop>);
  for (auto [entity, anim, drawable] : view.each()) {
    // dataKey・currentClip は実行中に変わるため、毎フレーム引いて確かめる
    const auto dataIt = dataRegistry.find(anim.dataKey);
    if (dataIt == dataRegistry.end()) {
      throw Error{U"AnimationDataRegistry にデータキーがない"};
    }
    const auto& data = dataIt->second;
    const auto clipIt = data.clips.find(anim.currentClip);
    if (clipIt == data.clips.end()) {
      throw Error{U"アニメーションデータにクリップがない"};
    }
    const auto& clip = clipIt->second;

    auto* td = std::get_if<TextureDrawable>(&drawable);
    if (td == nullptr) {
      throw Error{U"SpriteAnimation の Drawable が TextureDrawable でない"};
    }

    anim.elapsed = clip.advance(anim.elapsed, dt);
    const int32 col = clip.columnAt(anim.elapsed);
    const Point cell{col, clip.row};
    // data.textureKey の登録・読み込み済みと、クリップの count・speed が
    // 正であることは、ロード時（GameSetup.cpp・AnimationData::FromToml）が
    // 保証する
    auto region = TextureAsset{data.textureKey}(cell * data.size, data.size);

    if (anim.facingRight) {
      region = region.mirrored();
    }

    td->region = region;
    td->drawOffset = data.drawOffset;
  }
}
