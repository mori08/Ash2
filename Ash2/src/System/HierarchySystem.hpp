#pragma once
#include <entt/entt.hpp>

#include "Component/Hierarchy.hpp"

namespace HierarchySystem {

/// @brief registry に Hierarchy 削除シグナルを登録する
///
/// Hierarchy コンポーネントが削除されるタイミングで自動的に Hierarchy::Detach
/// が呼ばれ、 親・兄弟の連結リストが常に整合状態に保たれる。
inline void Connect(entt::registry& registry) {
  registry.on_destroy<Hierarchy>().connect<&Hierarchy::Detach>();
}

}  // namespace HierarchySystem
