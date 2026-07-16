#pragma once
#include <Siv3D.hpp>

/// @brief スプライトシート上の1アニメーションクリップ
struct AnimationClip {
  /// スプライトシート上の行番号（0始まり）
  int32 row;
  int32 count;
  /// コマ/秒
  double speed;
};

/// @brief アニメーション共有データ（エンティティ種別ごとに1つ）
struct AnimationData {
  /// TextureAsset キー（例: `assets/images/player.png`）
  String textureKey;
  Size size;
  /// TextureDrawable::anchor が示す位置からのずれ
  Vec2 drawOffset;
  /// クリップ名 → AnimationClip の対応表
  HashTable<String, AnimationClip> clips;

  /// @brief TOML からアニメーションデータを生成する
  [[nodiscard]] static AnimationData FromToml(const TOMLValue& toml);
};

/// @brief アニメーションデータレジストリ（registry.ctx() に格納）
using AnimationDataRegistry = HashTable<String, AnimationData>;
