#pragma once

/// @brief 透過しながら消滅する途中であることを示すコンポーネント
struct FadeOut {
  /// フェード全体の長さ（秒）
  double duration = 0.0;
  /// 残り時間（秒）
  double remaining = 0.0;
};
