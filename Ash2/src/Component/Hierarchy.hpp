#pragma once
#include <entt/entt.hpp>

#include "Component/LocalOffset.hpp"

/// @brief 親子関係を双方向連結リストで管理するコンポーネント
///
/// メンバの更新は static メンバ関数経由のみとし、不整合を防ぐ。
class Hierarchy {
 public:
  /// @return なければ entt::null
  [[nodiscard]] entt::entity parent() const { return m_parent; }
  /// @return なければ entt::null
  [[nodiscard]] entt::entity firstChild() const { return m_firstChild; }
  /// @return なければ entt::null
  [[nodiscard]] entt::entity nextSibling() const { return m_nextSibling; }
  /// @return なければ entt::null
  [[nodiscard]] entt::entity prevSibling() const { return m_prevSibling; }

  /// @brief 子を親にアタッチする（先頭に O(1) 挿入）
  ///
  /// 子がすでに別の親を持つ場合は先に Detach してから挿入する。
  /// parent・child が Hierarchy を持たない場合は自動的に追加する。
  /// @param offset 親からの相対座標（省略時はゼロ）
  static void Attach(
      entt::registry& registry, entt::entity parent, entt::entity child,
      LocalOffset offset = {}
  );

  /// @brief 子を親から切り離す（O(1)）
  static void Detach(entt::registry& registry, entt::entity child);

  /// @brief エンティティと全子孫を再帰的に破棄する
  static void DestroyWithChildren(
      entt::registry& registry, entt::entity entity
  );

 private:
  entt::entity m_parent = entt::null;
  entt::entity m_firstChild = entt::null;
  entt::entity m_nextSibling = entt::null;
  entt::entity m_prevSibling = entt::null;
};
