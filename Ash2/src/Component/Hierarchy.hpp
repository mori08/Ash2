#pragma once
#include <ThirdParty/entt/entt.hpp>

#include "Component/WorldPos.hpp"

/// @brief 親子関係を双方向連結リストで管理するコンポーネント
/// @details メンバの更新は static メンバ関数経由のみとし、不整合を防ぐ
class Hierarchy {
 public:
  /// @return 親エンティティ（なければ entt::null）
  [[nodiscard]] entt::entity parent() const { return m_parent; }
  /// @return 最初の子エンティティ（なければ entt::null）
  [[nodiscard]] entt::entity firstChild() const { return m_firstChild; }
  /// @return 次の兄弟エンティティ（なければ entt::null）
  [[nodiscard]] entt::entity nextSibling() const { return m_nextSibling; }
  /// @return 前の兄弟エンティティ（なければ entt::null）
  [[nodiscard]] entt::entity prevSibling() const { return m_prevSibling; }

  /// @brief 子を親にアタッチする（先頭に O(1) 挿入）
  /// @param registry ECS レジストリ
  /// @param parent   親エンティティ（Hierarchy がなければ自動追加）
  /// @param child    子エンティティ（Hierarchy がなければ自動追加）
  /// @param offset   親からの相対座標（省略時はゼロ）
  static void Attach(entt::registry& registry, entt::entity parent,
                     entt::entity child, WorldPos offset = {});

  /// @brief 子を親から切り離す（O(1)）
  /// @param registry ECS レジストリ
  /// @param child    切り離す子エンティティ
  static void Detach(entt::registry& registry, entt::entity child);

  /// @brief エンティティと全子孫を再帰的に破棄する
  /// @param registry ECS レジストリ
  /// @param entity   破棄するエンティティ
  static void DestroyWithChildren(entt::registry& registry,
                                  entt::entity entity);

 private:
  /// 親エンティティ
  entt::entity m_parent = entt::null;
  /// 最初の子エンティティ
  entt::entity m_firstChild = entt::null;
  /// 次の兄弟エンティティ
  entt::entity m_nextSibling = entt::null;
  /// 前の兄弟エンティティ
  entt::entity m_prevSibling = entt::null;
};
