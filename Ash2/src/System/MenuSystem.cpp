#include "System/MenuSystem.hpp"

#include "Component/Hierarchy.hpp"
#include "Component/Menu.hpp"
#include "Component/ScreenPos.hpp"

namespace {

/// @brief entity の親を返す
/// @return Hierarchy を持たなければ entt::null
entt::entity ParentOf(const entt::registry& registry, entt::entity entity) {
  // 移動先の書き間違いで Hierarchy を持たないエンティティが入りうる
  const auto* hierarchy = registry.try_get<Hierarchy>(entity);
  return (hierarchy != nullptr) ? hierarchy->parent() : entt::null;
}

/// @brief 選択中の子を返す
/// @return なければ entt::null
entt::entity FindFocused(
    const entt::registry& registry, entt::entity menuEntity
) {
  const auto children = Hierarchy::Children(registry, menuEntity);
  const auto it = std::ranges::find_if(children, [&](entt::entity child) {
    return registry.all_of<Focused>(child);
  });
  return (it != std::default_sentinel) ? *it : entt::null;
}

/// @brief 選択を target に移す。entt::null なら選択なしにする
/// @throw Error target が null でなく、メニューの子でないとき
void Focus(
    entt::registry& registry, entt::entity menuEntity, entt::entity target
) {
  // 全ボタンを設定し終えるまで検査できないため、使う瞬間に確かめる
  if (target != entt::null && ParentOf(registry, target) != menuEntity) {
    throw Error{U"選択の移動先がメニューの子でない"};
  }
  // 1メニューに1つだけになるよう、先に全員から外す
  for (const entt::entity child : Hierarchy::Children(registry, menuEntity)) {
    registry.remove<Focused>(child);
  }
  if (target != entt::null) {
    registry.emplace<Focused>(target);
  }
}

/// @brief pos の下にあるボタンを返す
/// @return なければ entt::null
/// @throw Error 子が Button と ScreenPos を持たないとき
entt::entity HitTest(
    const entt::registry& registry, entt::entity menuEntity, Vec2 pos
) {
  for (const entt::entity child : Hierarchy::Children(registry, menuEntity)) {
    if (!registry.all_of<Button, ScreenPos>(child)) {
      throw Error{U"メニューの子が Button と ScreenPos を持たない"};
    }
    const auto& button = registry.get<Button>(child);
    const auto& screenPos = registry.get<ScreenPos>(child);
    if (RectF{Arg::center(screenPos.pos), button.size}.intersects(pos)) {
      return child;
    }
  }
  return entt::null;
}

/// @brief 方向に対応する移動先を返す
entt::entity DestinationOf(const Button& button, NavDirection direction) {
  switch (direction) {
    case NavDirection::Up:
      return button.up;
    case NavDirection::Down:
      return button.down;
    case NavDirection::Left:
      return button.left;
    case NavDirection::Right:
      return button.right;
  }
  throw Error{U"NavDirection が未知の列挙値"};
}

}  // namespace

Optional<entt::entity> MenuSystem::Update(
    entt::registry& registry, const InputState& input, entt::entity menuEntity
) {
  if (!registry.valid(menuEntity) || !registry.all_of<Menu>(menuEntity)) {
    throw Error{U"menuEntity が Menu を持たない"};
  }

  if (input.pointerMoved) {
    Focus(
        registry, menuEntity, HitTest(registry, menuEntity, *input.pointerMoved)
    );
  }

  if (input.navigate) {
    const entt::entity focused = FindFocused(registry, menuEntity);
    if (focused == entt::null) {
      Focus(registry, menuEntity, registry.get<Menu>(menuEntity).defaultButton);
    } else {
      const entt::entity destination =
          DestinationOf(registry.get<Button>(focused), *input.navigate);
      if (destination != entt::null) {
        Focus(registry, menuEntity, destination);
      }
    }
  }

  Optional<entt::entity> pressed = none;
  if (input.confirmDown) {
    const entt::entity focused = FindFocused(registry, menuEntity);
    if (focused != entt::null) {
      pressed = focused;
    }
  }

  if (input.clickDown && input.pointerPos) {
    const entt::entity hit = HitTest(registry, menuEntity, *input.pointerPos);
    if (hit != entt::null) {
      // パッドで選んだ後に別のボタンをクリックした場合も、見た目を合わせる
      Focus(registry, menuEntity, hit);
      pressed = hit;
    }
  }

  return pressed;
}
