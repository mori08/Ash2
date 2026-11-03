#pragma once
#include <entt/entt.hpp>
#include <iterator>
#include <ranges>

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

  /// @brief 親の子を先頭から辿る forward iterator
  ///
  /// 辿っている間は連結を変更しない。
  /// 特に今指している子の Detach・破棄は禁止。
  /// registry より長くイテレータを保持してはならない。
  class ChildIterator {
   public:
    using iterator_concept = std::forward_iterator_tag;
    using value_type = entt::entity;
    using difference_type = std::ptrdiff_t;

    // メンバ初期化子ではなく初期化リストで書く。
    // 入れ子のクラスのメンバ初期化子は外側のクラスの定義中に使えず、
    // ChildRange（subrange）を作る時点で既定構築できないと判定されるため。
    ChildIterator()
        : m_registry{nullptr}, m_parent{entt::null}, m_current{entt::null} {}

    /// @brief parent の最初の子を指すイテレータを作る
    ///
    /// parent が Hierarchy を持たなければ終端を指す。
    ChildIterator(const entt::registry& registry, entt::entity parent);

    [[nodiscard]] entt::entity operator*() const { return m_current; }

    /// @throw Error 今の子が走査中に Detach されたとき
    ChildIterator& operator++();
    ChildIterator operator++(int) {
      auto before = *this;
      ++*this;
      return before;
    }

    [[nodiscard]] bool operator==(std::default_sentinel_t) const {
      return m_current == entt::null;
    }
    [[nodiscard]] bool operator==(const ChildIterator&) const = default;

   private:
    const entt::registry* m_registry;
    entt::entity m_parent;
    entt::entity m_current;
  };

  /// 子の範囲。`size()` は使えない
  using ChildRange =
      std::ranges::subrange<ChildIterator, std::default_sentinel_t>;

  /// @brief 親の子を先頭から辿る範囲を返す
  ///
  /// 親が Hierarchy を持たなければ空の範囲。
  /// 範囲を辿っている間は連結を変更しない。
  /// 辿りながら Detach・破棄するときは先に Array 等へ集める。
  [[nodiscard]] static ChildRange Children(
      const entt::registry& registry, entt::entity parent
  );

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
  ///
  /// 子を持つエンティティはこの関数で破棄する。
  /// `registry.destroy` で直接破棄すると、子が破棄済みの親を指したまま残る。
  /// @throw Error entity が無効なとき
  static void DestroyWithChildren(
      entt::registry& registry, entt::entity entity
  );

 private:
  entt::entity m_parent = entt::null;
  entt::entity m_firstChild = entt::null;
  entt::entity m_nextSibling = entt::null;
  entt::entity m_prevSibling = entt::null;
};
