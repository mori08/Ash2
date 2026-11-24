#pragma once

/// @brief 植物1株の揺れの状態（親エンティティに付ける。子は `PlantStem`）
///
/// 根元の位置は同じエンティティの `DrawPos` が持つ。
struct Plant {
  /// 1往復の秒数（0 より大きい）
  double period;
  /// 出だしのずれ。1周期を 1.0 とした割合
  double offset;
  /// 間隔の振れ幅。基準の間隔に対する割合
  double spreadAmplitude;
  /// 生成からの経過時間（秒）
  double elapsed = 0.0;
};

/// @brief 植物の棒1本（子エンティティに付ける）
struct PlantStem {
  /// 株の中心からの基準の横間隔（px。負が左）
  double dx;
};
