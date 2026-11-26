#pragma once

/// @brief 波紋の広がり方（`DrawPos`・`DrawColor`・`EllipseFrameDrawable`
/// と組で使う）
///
/// 寿命の間に、横半径を `startRadius` から `endRadius` へ広げながら薄くする。
struct Ripple {
  /// 出てからの経過秒数
  double elapsed;
  /// 消えるまでの秒数
  double life;
  /// 広がり始めの横半径（px。奥行きの倍率を掛け済み）
  double startRadius;
  /// 広がりきったときの横半径（px。奥行きの倍率を掛け済み）
  double endRadius;
  /// 縦半径 / 横半径
  double aspect;
};
