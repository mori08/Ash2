#include "System/MenuSystem.hpp"

#include "Component/Hierarchy.hpp"
#include "Component/Menu.hpp"
#include "Component/ScreenPos.hpp"

namespace {

/// @brief メニューの子を列挙する
Array<entt::entity> ChildrenOf(
    const entt::registry& registry, entt::entity menuEntity
) {
  Array<entt::entity> children;
  const auto* hierarchy = registry.try_get<Hierarchy>(menuEntity);
  if (hierarchy == nullptr) {
    return children;
  }
  for (entt::entity child = hierarchy->firstChild(); child != entt::null;
       child = registry.get<Hierarchy>(child).nextSibling()) {
    children.push_back(child);
  }
  return children;
}

/// @brief entity がメニューの子か
bool IsChildOf(const Array<entt::entity>& children, entt::entity entity) {
  return children.contains(entity);
}

/// @brief メニューの構造が契約を満たすか
// 全ボタンを設定し終えるまで検査できないため、Setup 側では行わない
bool IsValidStructure(
    const entt::registry& registry, entt::entity menuEntity,
    const Array<entt::entity>& children
) {
  if (!registry.valid(menuEntity) || !registry.all_of<Menu>(menuEntity)) {
    return false;
  }
  if (!IsChildOf(children, registry.get<Menu>(menuEntity).defaultButton)) {
    return false;
  }
  for (const entt::entity child : children) {
    if (!registry.all_of<Button, ScreenPos>(child)) {
      return false;
    }
    const auto& button = registry.get<Button>(child);
    for (const entt::entity target :
         {button.up, button.down, button.left, button.right}) {
      if (target != entt::null && !IsChildOf(children, target)) {
        return false;
      }
    }
  }
  return true;
}

/// @brief 選択中の子を返す
/// @return なければ entt::null
entt::entity FindFocused(
    const entt::registry& registry, const Array<entt::entity>& children
) {
  for (const entt::entity child : children) {
    if (registry.all_of<Focused>(child)) {
      return child;
    }
  }
  return entt::null;
}

/// @brief 選択を target に移す。entt::null なら選択なしにする
void Focus(
    entt::registry& registry, const Array<entt::entity>& children,
    entt::entity target
) {
  // 1メニューに1つだけになるよう、先に全員から外す
  for (const entt::entity child : children) {
    registry.remove<Focused>(child);
  }
  if (target != entt::null) {
    registry.emplace<Focused>(target);
  }
}

/// @brief pos の下にあるボタンを返す
/// @return なければ entt::null
entt::entity HitTest(
    const entt::registry& registry, const Array<entt::entity>& children,
    Vec2 pos
) {
  for (const entt::entity child : children) {
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
  return entt::null;
}

}  // namespace

Optional<entt::entity> MenuSystem::Update(
    entt::registry& registry, const InputState& input, entt::entity menuEntity
) {
  const Array<entt::entity> children = ChildrenOf(registry, menuEntity);
  if (!IsValidStructure(registry, menuEntity, children)) {
    throw Error{
        U"メニューの構造が不正（子が Button と ScreenPos を持たない、"
        U"または defaultButton・移動先が子でない）"
    };
  }

  if (input.pointerMoved) {
    Focus(registry, children, HitTest(registry, children, *input.pointerMoved));
  }

  if (input.navigate) {
    const entt::entity focused = FindFocused(registry, children);
    if (focused == entt::null) {
      Focus(registry, children, registry.get<Menu>(menuEntity).defaultButton);
    } else {
      const entt::entity destination =
          DestinationOf(registry.get<Button>(focused), *input.navigate);
      if (destination != entt::null) {
        Focus(registry, children, destination);
      }
    }
  }

  Optional<entt::entity> pressed = none;
  if (input.confirmDown) {
    const entt::entity focused = FindFocused(registry, children);
    if (focused != entt::null) {
      pressed = focused;
    }
  }

  if (input.clickDown && input.pointerPos) {
    const entt::entity hit = HitTest(registry, children, *input.pointerPos);
    if (hit != entt::null) {
      // パッドで選んだ後に別のボタンをクリックした場合も、見た目を合わせる
      Focus(registry, children, hit);
      pressed = hit;
    }
  }

  return pressed;
}
