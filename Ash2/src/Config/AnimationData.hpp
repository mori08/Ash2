#pragma once
#include <Siv3D.hpp>

/// @brief スプライトシート上の1アニメーションクリップ
struct AnimationClip {
  /// スプライトシート上の行番号（0始まり）
  int row;
  int count;
  /// コマ/秒
  double speed;
};

/// @brief アニメーション共有データ（エンティティ種別ごとに1つ）
struct AnimationData {
  /// TextureAsset キー（例: `assets/images/player.png`）
  s3d::String textureKey;
  s3d::Size size;
  /// TextureDrawable::anchor が示す位置からのずれ
  s3d::Vec2 drawOffset;
  /// クリップ名 → AnimationClip の対応表
  s3d::HashTable<s3d::String, AnimationClip> clips;

  /// @brief TOML からアニメーションデータを生成する
  [[nodiscard]] static AnimationData FromToml(const s3d::TOMLValue& toml);
};

/// @brief アニメーションデータレジストリ（registry.ctx() に格納）
using AnimationDataRegistry = s3d::HashTable<s3d::String, AnimationData>;
