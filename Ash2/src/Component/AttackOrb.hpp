#pragma once
#include <Siv3D.hpp>

/// @brief プレイヤーの攻撃判定・見た目用の珠エンティティであることを示すタグ
///
/// `Hierarchy` で所有者（プレイヤー）の子としてアタッチされる。中断経路
/// （着地・被弾）で所有者の子を走査し一括解放する際の識別に使う
/// （`PlayerMotion::ReleaseAttackOrbs` 参照）。役割は必ず
/// `AttackHitboxOrb`/`AttackLightOrb` のいずれかと組み合わせて表す。
struct AttackOrb {};

/// @brief 攻撃判定を担う珠であることを示すタグ
struct AttackHitboxOrb {};

/// @brief 見た目だけを担う光の珠であることを示すタグ
struct AttackLightOrb {
  /// 扇の並び順（0始まり）。offsetFn へ渡して上下の広がりを決める
  int32 index = 0;
};
