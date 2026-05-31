#pragma once

/// @brief 攻撃中コンポーネント（タグ兼攻撃力）
/// このコンポーネントを持つエンティティが Collider
/// も持つとき、攻撃判定が有効になる
struct Attack {
  /// 与えるダメージ量
  int damage;
};
