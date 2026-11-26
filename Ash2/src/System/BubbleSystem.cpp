#include "System/BubbleSystem.hpp"

#include "Component/Bubble.hpp"
#include "Component/DrawColor.hpp"
#include "Component/DrawPos.hpp"
#include "Component/Drawable.hpp"

namespace {

/// @brief popPos で割れる泡を作る
void CreateBubble(
    entt::registry& registry, const BubbleConfig& config, const RectF& area,
    const Vec2& popPos
) {
  // 水面（area の上端）で farScale、下端で 1
  const double depthRate = (popPos.y - area.y) / area.h;
  const double scale = Math::Lerp(config.farScale, 1.0, depthRate);
  const double startY = popPos.y + config.rise * scale;

  const auto entity = registry.create();
  registry.emplace<Bubble>(
      entity,
      Bubble{
          .popY = popPos.y,
          .startY = startY,
          .depthRate = depthRate,
          .speed = config.speed * scale,
          .popAlpha = config.popAlpha
      }
  );
  // 奥の泡から順に描く
  registry.emplace<DrawPos>(
      entity,
      DrawPos{
          .pos = Vec2{popPos.x, startY},
          .layer = DrawLayer::kBackground,
          .depth = -popPos.y
      }
  );
  registry.emplace<Drawable>(entity, CircleDrawable{.radius = config.radius});
  registry.emplace<DrawColor>(
      entity, DrawColor{.color = ColorF{config.color, 0.0}}
  );
}

}  // namespace

void BubbleSystem::Spawn(
    entt::registry& registry, const FrameData& frameData,
    const BubbleConfig& config, const RectF& area
) {
  const Vec2 popPos{
      frameData.random.range(area.x, area.x + area.w),
      frameData.random.range(area.y + config.popMargin, area.y + area.h)
  };
  CreateBubble(registry, config, area, popPos);
}

Array<BubblePop> BubbleSystem::Update(
    entt::registry& registry, const FrameData& frameData
) {
  Array<entt::entity> popped;
  Array<BubblePop> pops;
  for (const auto [entity, bubble, drawPos, drawColor] :
       registry.view<const Bubble, DrawPos, DrawColor>().each()) {
    drawPos.pos.y -= bubble.speed * frameData.dt;
    if (drawPos.pos.y <= bubble.popY) {
      popped.push_back(entity);
      pops.push_back(
          BubblePop{
              .pos = Vec2{drawPos.pos.x, bubble.popY},
              .depthRate = bubble.depthRate
          }
      );
      continue;
    }

    // 生まれた位置で 0、割れる位置で 1
    const double progress =
        (bubble.startY - drawPos.pos.y) / (bubble.startY - bubble.popY);
    drawColor.color.a = bubble.popAlpha * progress * progress;
  }
  for (const auto entity : popped) {
    registry.destroy(entity);
  }
  return pops;
}
