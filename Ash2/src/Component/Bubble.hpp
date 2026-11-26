#pragma once

/// @brief 泡の昇り方（`DrawPos`・`DrawColor`・`CircleDrawable` と組で使う）
///
/// 真上に昇り、`popY` に着いたら割れる。
struct Bubble {
  /// 割れる y 座標（画面座標、px）
  double popY;
  /// 生まれた y 座標（画面座標、px）。`popY` より下にある
  double startY;
  /// 割れる位置の奥行き。水面で 0、画面の下端で 1
  double depthRate;
  /// 昇る速さ（px/秒。奥行きで縮小済み）
  double speed;
  /// 割れるときの不透明度（生まれたときは 0）
  double popAlpha;
};
