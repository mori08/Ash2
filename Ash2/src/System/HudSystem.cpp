#include "System/HudSystem.hpp"

#include "Component/DrawColor.hpp"
#include "Component/Drawable.hpp"
#include "Component/Hp.hpp"
#include "Component/Player.hpp"
#include "Component/ScreenPos.hpp"
#include "Component/Stamina.hpp"
#include "System/DrawShape.hpp"

namespace {

/// @brief Player + Hp + Stamina を持つ最初の1体の HP / スタミナゲージを
/// 画面左上に描画する
void DrawGauges(const entt::registry& registry) {
  constexpr double kBarX = 16.0;
  constexpr double kHpBarY = 16.0;
  constexpr double kStaminaBarY = 40.0;
  constexpr double kBarWidth = 200.0;
  constexpr double kBarHeight = 18.0;
  constexpr ColorF kBgColor{0.2, 0.2, 0.2, 0.7};
  constexpr ColorF kHpColor{0.2, 0.8, 0.2};
  constexpr ColorF kStaminaColor{0.9, 0.8, 0.1};

  const auto view = registry.view<const Player, const Hp, const Stamina>();
  const auto entity = view.front();
  if (entity == entt::null) return;

  const auto& hp = view.get<const Hp>(entity);
  const auto& stamina = view.get<const Stamina>(entity);

  RectF{kBarX, kHpBarY, kBarWidth, kBarHeight}.draw(kBgColor);
  if (hp.max > 0) {
    const double hpRatio =
        Clamp(static_cast<double>(hp.current) / hp.max, 0.0, 1.0);
    RectF{kBarX, kHpBarY, kBarWidth * hpRatio, kBarHeight}.draw(kHpColor);
  }

  RectF{kBarX, kStaminaBarY, kBarWidth, kBarHeight}.draw(kBgColor);
  if (stamina.max > 0) {
    const double staminaRatio =
        Clamp(static_cast<double>(stamina.current) / stamina.max, 0.0, 1.0);
    RectF{kBarX, kStaminaBarY, kBarWidth * staminaRatio, kBarHeight}.draw(
        kStaminaColor
    );
  }
}

/// @brief ScreenPos + Drawable を (layer, entity) の昇順で描画する
///
/// storage の走査順は削除で入れ替わるため、ソートして毎フレーム同じ順にする。
/// layer が同値のものは entity 昇順にする。
void DrawScreenEntities(const entt::registry& registry) {
  struct Entry {
    entt::entity entity;
    std::reference_wrapper<const ScreenPos> pos;
    std::reference_wrapper<const Drawable> drawable;
    ColorF color;
  };

  Array<Entry> entries;
  for (const auto& [entity, pos, drawable] :
       registry.view<const ScreenPos, const Drawable>().each()) {
    const auto* drawColor = registry.try_get<DrawColor>(entity);
    entries.push_back(
        {.entity = entity,
         .pos = std::cref(pos),
         .drawable = std::cref(drawable),
         .color = (drawColor != nullptr) ? drawColor->color : kDefaultDrawColor}
    );
  }

  std::ranges::sort(entries, {}, [](const Entry& entry) {
    return std::pair{entry.pos.get().layer, entry.entity};
  });

  for (const auto& entry : entries) {
    DrawShape(entry.drawable.get(), entry.pos.get().pos, entry.color);
  }
}

}  // namespace

void HudSystem::Draw(const entt::registry& registry) {
  DrawGauges(registry);
  DrawScreenEntities(registry);
}
