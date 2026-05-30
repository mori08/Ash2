#pragma once

#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief アニメーション再生確認フェーズ
class AnimationViewerPhase : public IPhase {
 public:
  /// @brief AnimationViewerPhase の生成パラメータ
  struct Param {
    /// AnimationDataRegistry のキー（エンティティ種別名）
    s3d::String dataKey;
    /// 初期クリップ名（空文字の場合は最初のクリップを使用）
    s3d::String initialClip;
  };

  /// @brief コンストラクタ
  /// @param param 生成パラメータ
  explicit AnimationViewerPhase(const Param& param);

  /// @brief 一時エンティティを生成する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @param frameData フレームごとの更新データ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

  /// @brief 一時エンティティを破棄する
  /// @param registry ECS レジストリ
  void onBeforePop(entt::registry& registry) override;

 private:
  static constexpr int KFontSize = 20;

  /// AnimationDataRegistry のキー
  s3d::String m_dataKey;
  /// 表示対象エンティティ
  entt::entity m_entity = entt::null;
  /// クリップ名の配列（ソート済み）
  s3d::Array<s3d::String> m_clips;
  /// 現在のクリップインデックス
  int m_clipIndex = 0;
  /// 描画フォント
  s3d::Font m_font{KFontSize};
};
