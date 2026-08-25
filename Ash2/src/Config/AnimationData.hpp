#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief スプライトシート上の1アニメーションクリップ
struct AnimationClip {
  /// スプライトシート上の行番号（0始まり）
  int32 row;
  int32 count;
  /// コマ/秒
  double speed;
  /// 末尾まで再生したら先頭へ戻るか。false は最終コマで停止する
  bool loop = false;

  /// @brief 全コマを1周する時間（秒）
  [[nodiscard]] double cycleDuration() const {
    return static_cast<double>(count) / speed;
  }
  /// @brief dt 秒進めた位相を返す
  /// @return loop なら [0, cycleDuration()) へラップ、そうでなければ
  ///         cycleDuration() でクランプした値
  [[nodiscard]] double advance(double elapsed, double dt) const {
    const double next = elapsed + dt;
    return loop ? Math::Fmod(next, cycleDuration())
                : Min(next, cycleDuration());
  }
  /// @brief 位相に対応するコマ番号（列）
  /// @return [0, count - 1] にクランプした値
  [[nodiscard]] int32 columnAt(double elapsed) const {
    return Clamp(static_cast<int32>(elapsed * speed), 0, count - 1);
  }
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
  [[nodiscard]] static std::expected<AnimationData, String> FromToml(
      const TOMLValue& toml
  );
};

/// @brief アニメーションデータレジストリ（registry.ctx() に格納）
using AnimationDataRegistry = HashTable<String, AnimationData>;
