#pragma once
#include <Siv3D.hpp>

/// @brief スプライトシート上の1アニメーションクリップ
struct AnimationClip {
  /// スプライトシート上の行番号（0始まり）
  int row;
  /// コマ数
  int count;
  /// 再生速度（コマ/秒）
  double speed;
};

/// @brief アニメーション共有データ（エンティティ種別ごとに1つ）
struct AnimationData {
  /// TextureAsset キー（例: `assets/images/player.png`）
  s3d::String textureKey;
  /// 1コマのサイズ（幅・高さ）
  s3d::Size size;
  /// 描画オフセット（中心座標からのずれ）
  s3d::Vec2 drawOffset;
  /// クリップ名 → AnimationClip の対応表
  s3d::HashTable<s3d::String, AnimationClip> clips;

  /// @brief TOML からアニメーションデータを生成する
  /// @param toml TOML 値
  /// @return AnimationData
  [[nodiscard]] static AnimationData FromToml(const s3d::TOMLValue& toml);
};

/// @brief アニメーションデータレジストリ（registry.ctx() に格納）
using AnimationDataRegistry = s3d::HashTable<s3d::String, AnimationData>;
