#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief 攻撃中コンポーネント（タグ兼攻撃力）
///
/// このコンポーネントを持つエンティティが `Collider`
/// も持つとき、攻撃判定が有効になる。
struct Attack {
  /// 与えるダメージ量
  int damage = 0;

  /// 複数コライダー構成時のルートエンティティ（単体の場合は entt::null）
  entt::entity root = entt::null;

  /// 攻撃の生存期間中にヒット済みのターゲット（重複ヒット防止用）
  s3d::HashSet<entt::entity> hitTargets;
};
