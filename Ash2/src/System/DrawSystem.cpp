#include "System/DrawSystem.hpp"

#include "Component/Drawable.hpp"
#include "Component/WorldPos.hpp"
#include "Util/Overloaded.hpp"

void DrawSystem::Draw(const entt::registry& registry) {
  const Vec2 cameraOffset = Scene::Center();

  struct DrawEntry {
    std::reference_wrapper<const WorldPos> pos;
    std::reference_wrapper<const Drawable> drawable;
  };

  Array<DrawEntry> entries;
  for (const auto& [entity, pos, drawable] :
       registry.view<const WorldPos, const Drawable>().each()) {
    entries.push_back({std::cref(pos), std::cref(drawable)});
  }

  std::ranges::sort(entries, [](const DrawEntry& a, const DrawEntry& b) {
    return DrawOrderLess(a.pos.get(), b.pos.get());
  });

  for (const auto& entry : entries) {
    const Vec2 screenPos = cameraOffset + entry.pos.get().toScreen();
    std::visit(
        Overloaded{
            [&screenPos](const RectDrawable& shape) {
              // WorldPos = 接地点（足元）のため、下端中央を基準に描画する
              const RectF rect{Arg::bottomCenter(screenPos), shape.size.x,
                               shape.size.y};
              rect.draw(shape.color);
              if (shape.border) {
                rect.drawFrame(shape.border->thickness, shape.border->color);
              }
            },
            [&screenPos](const CircleDrawable& shape) {
              // 弾の Collider は WorldPos と一致する点として定義され、
              // 円は中心対称なので中心描画のままで判定と一致する
              const Circle circle{screenPos, shape.radius};
              circle.draw(shape.color);
              if (shape.border) {
                circle.drawFrame(shape.border->thickness, shape.border->color);
              }
            },
            [&screenPos](const PieDrawable& shape) {
              // 現状未使用。将来 WorldPos に合わせる場合は描画方法を再検討する
              const Circle circle{screenPos, shape.radius};
              circle.drawPie(shape.startAngle, shape.angle, shape.color);
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
            [&screenPos](const TextureDrawable& shape) {
              // WorldPos = 接地点（足元）のため、下端中央を基準に描画する
              shape.region.draw(
                  Arg::bottomCenter(screenPos + shape.drawOffset));
            },
        },
        entry.drawable.get());
  }
}
