#include "System/DrawSystem.hpp"

#include "Component/Drawable.hpp"
#include "WorldPos.hpp"

namespace {
template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
}  // namespace

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
              RectF{Arg::bottomCenter(screenPos), shape.size.x, shape.size.y}
                  .draw(shape.color);
            },
        },
        entry.drawable.get());
  }
}
