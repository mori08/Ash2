#pragma once

#include <Siv3D.hpp>

#include "IPhase.hpp"

/// @brief テストフェーズ一覧メニュー
class TestMenuPhase : public IPhase {
 public:
  /// @brief TestMenuPhase の生成パラメータ（引数なし）
  struct Param {};

  /// @brief デフォルトコンストラクタ
  TestMenuPhase() = default;

  /// @brief コンストラクタ（Param 受け取り版）
  /// @param param 生成パラメータ（未使用）
  explicit TestMenuPhase(const Param& /*param*/) : TestMenuPhase() {}

  /// @brief メニュー項目を初期化する
  /// @param registry ECS レジストリ
  void onAfterPush(entt::registry& registry) override;

  /// @brief 毎フレームの更新処理
  /// @param registry ECS レジストリ
  /// @param frameData フレームごとの更新データ
  /// @return フェーズスタックへの操作
  [[nodiscard]] PhaseCommand update(entt::registry& registry,
                                    const FrameData& frameData) override;

 private:
  /// メニュー項目（表示名 + 生成ラムダ）
  struct MenuItem {
    /// 表示名
    s3d::String label;
    /// フェーズ生成ラムダ
    std::function<std::unique_ptr<IPhase>(entt::registry&)> create;
  };

  static constexpr int KFontSize = 24;

  /// メニュー項目リスト
  s3d::Array<MenuItem> m_items;
  /// 現在の選択インデックス
  int m_selectedIndex = 0;
  /// 描画フォント
  s3d::Font m_font{KFontSize};
};
