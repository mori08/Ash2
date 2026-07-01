#pragma once
#include <Siv3D.hpp>

/// @brief ひるみリアクション中であることを示すタイマーコンポーネント
///
/// `StaggerSystem` が経過時間を減算し、0 以下になった時点で
/// `RectDrawable::size` を `originalSize` に戻したうえで除去する。
struct Stagger {
  /// 残り時間（秒）
  double remaining = 0.0;
  /// 合計時間（秒）。縮み量の正規化に使う
  double duration = 0.0;
  /// 付与前の RectDrawable::size
  SizeF originalSize;
};
