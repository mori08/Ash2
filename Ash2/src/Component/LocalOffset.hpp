#pragma once

/// @brief 親からの相対座標
///
/// Hierarchy を持つエンティティに付ける。
/// @note このコンポーネントを持つエンティティの WorldPos は AttachmentSystem が
/// 毎フレーム上書きするため、直接書き換えても反映されない
struct LocalOffset {
  /// 横方向のずれ
  double w = 0.0;
  /// 高さ方向のずれ（上方向が正）
  double h = 0.0;
  /// 奥行き方向のずれ
  double d = 0.0;
};
