#include "System/DrawSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/WorldPos.hpp"
#include "Util/Overloaded.hpp"

void DrawSystem::Draw(const entt::registry& registry) {
  // HUD・フォント描画へ波及させないため、この関数のスコープに閉じる
  const ScopedRenderStates2D sampler{SamplerState::ClampNearest};
  const Vec2 cameraOffset = Scene::Center();

  struct DrawEntry {
    std::reference_wrapper<const WorldPos> pos;
    std::reference_wrapper<const Drawable> drawable;
    ColorF color;
  };

  Array<DrawEntry> entries;
  for (const auto& [entity, pos, drawable] :
       registry.view<const WorldPos, const Drawable>().each()) {
    const auto* drawColor = registry.try_get<DrawColor>(entity);
    entries.push_back(
        {.pos = std::cref(pos),
         .drawable = std::cref(drawable),
         .color = (drawColor != nullptr) ? drawColor->color : kDefaultDrawColor}
    );
  }

  std::ranges::sort(entries, [](const DrawEntry& a, const DrawEntry& b) {
    return DrawOrderLess(a.pos.get(), b.pos.get());
  });

  for (const auto& entry : entries) {
    const Vec2 screenPos = cameraOffset + entry.pos.get().toScreen();
    const ColorF& color = entry.color;
    std::visit(
        Overloaded{
            [&screenPos, &color](const RectDrawable& shape) {
              const RectF rect = [&] {
                switch (shape.anchor) {
                  case DrawAnchor::BottomCenter:
                    return RectF{
                        Arg::bottomCenter(screenPos), shape.size.x, shape.size.y
                    };
                  case DrawAnchor::Center:
                  default:
                    return RectF{
                        Arg::center(screenPos), shape.size.x, shape.size.y
                    };
                }
              }();
              rect.draw(color);
            },
            [&screenPos, &color](const CircleDrawable& shape) {
              const Circle circle{screenPos, shape.radius};
              circle.draw(color);
            },
            [&screenPos, &color](const TextureDrawable& shape) {
              // 図形は AA が効くので丸めない。テクスチャのみ整数化する
              const Vec2 anchorPos = Math::Round(screenPos + shape.drawOffset);
              switch (shape.anchor) {
                case DrawAnchor::BottomCenter:
                  shape.region.draw(Arg::bottomCenter(anchorPos), color);
                  break;
                case DrawAnchor::Center:
                default:
                  shape.region.draw(Arg::center(anchorPos), color);
                  break;
              }
            },
        },
        entry.drawable.get()
    );
  }
}
