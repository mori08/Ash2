#pragma once
#include <Siv3D.hpp>

/// @brief プレイヤーの攻撃判定・見た目用の珠エンティティであることを示すタグ
///
/// `Hierarchy` で所有者（プレイヤー）の子としてアタッチされる。中断経路
/// （着地・被弾）で所有者の子を走査し一括解放する際の識別に使う
/// （`PlayerMotion::ReleaseAttackOrbs` 参照）。
struct AttackOrb {
  /// 生成順のインデックス（0始まり）。デバッグ表示や将来の演出向けの識別子で、
  /// 解放処理自体はこの値を参照しない
  int32 index = 0;
};
