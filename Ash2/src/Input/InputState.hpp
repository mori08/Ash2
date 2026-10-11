#pragma once
#include <Siv3D.hpp>

/// @brief メニュー操作の方向
enum class NavDirection : uint8 { Up, Down, Left, Right };

/// @brief フレームごとのプレイヤー入力状態
/// @note `Key` や `TOMLValue` のようなテストしづらい型は持ち込まない方針だが、
/// `Vec2` のような単純な数学型は許容する。そのフレームに起きた操作だけを持ち、
/// メニューの選択状態のような持ち越す状態は持たない
struct InputState {
  /// 移動軸入力（x: 横方向 w 軸・右が正、y: 奥行き方向 d 軸・奥が正）。
  /// 常に長さ 1.0 以下に正規化されていることが保証される
  Vec2 moveAxis = Vec2::Zero();
  bool jumpDown = false;
  bool attackDown = false;
  bool rangedAttackDown = false;
  bool dashDown = false;
  /// ロック方向入力（画面座標系。右が正・下が正）。デッドゾーンは掛けない
  /// @note moveAxis.y は奥行き d（奥が正）で、y の意味が異なる
  Vec2 lockAxis = Vec2::Zero();
  /// マウスカーソルの画面座標。ゲームパッド操作中は none
  Optional<Vec2> pointerPos = none;
  /// メニューの方向入力。押した瞬間のフレームだけ値を持つ
  Optional<NavDirection> navigate = none;
  /// メニューの決定入力（そのフレームに押された）
  bool confirmDown = false;
  /// マウス左ボタンがそのフレームに押された。位置は `pointerPos` を使う
  bool clickDown = false;
  /// カーソルがそのフレームに動いたときの画面座標。止まっていれば none
  Optional<Vec2> pointerMoved = none;
};
