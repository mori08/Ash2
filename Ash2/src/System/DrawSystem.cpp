#include "System/DrawSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/WorldPos.hpp"
#include "Util/Overloaded.hpp"

void DrawSystem::Draw(const entt::registry& registry) {
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
        {std::cref(pos), std::cref(drawable),
         (drawColor != nullptr) ? drawColor->color : KDefaultDrawColor});
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
                    return RectF{Arg::bottomCenter(screenPos), shape.size.x,
                                 shape.size.y};
                  case DrawAnchor::Center:
                  default:
                    return RectF{Arg::center(screenPos), shape.size.x,
                                 shape.size.y};
                }
              }();
              rect.draw(color);
              if (shape.border) {
                rect.drawFrame(shape.border->thickness, shape.border->color);
              }
            },
            [&screenPos, &color](const CircleDrawable& shape) {
              const Circle circle{screenPos, shape.radius};
              circle.draw(color);
              if (shape.border) {
                circle.drawFrame(shape.border->thickness, shape.border->color);
              }
            },
            [&screenPos, &color](const PieDrawable& shape) {
              const Circle circle{screenPos, shape.radius};
              circle.drawPie(shape.startAngle, shape.angle, color);
              if (shape.border) {
                const auto& b = *shape.border;
                circle.drawArc(shape.startAngle, shape.angle, 0.0, b.thickness,
                               b.color);
                const Vec2 p1 =
                    screenPos + Vec2{Circular{shape.radius, shape.startAngle}};
                const Vec2 p2 =
                    screenPos + Vec2{Circular{shape.radius,
                                              shape.startAngle + shape.angle}};
                Line{screenPos, p1}.draw(b.thickness, b.color);
                Line{screenPos, p2}.draw(b.thickness, b.color);
              }
            },
            [&screenPos, &color](const TextureDrawable& shape) {
              const Vec2 anchorPos = screenPos + shape.drawOffset;
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
        entry.drawable.get());
  }
}
