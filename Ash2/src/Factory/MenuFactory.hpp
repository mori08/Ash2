#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief メニューとボタンを設定するファクトリ
/// @note `Create` は持たない。エンティティは呼び出し側が先に作る
class MenuFactory {
 public:
  /// @brief SetupButton の設定パラメータ
  struct ButtonParam {
    /// 画面上の中心位置（px）
    Vec2 pos{};
    /// 当たり範囲の大きさ（px）
    SizeF size{};
    /// `ScreenPos::layer`
    int32 layer = 0;
    /// 各方向の移動先。同じメニューの子に限る。ない方向は entt::null
    entt::entity up = entt::null;
    entt::entity down = entt::null;
    entt::entity left = entt::null;
    entt::entity right = entt::null;
  };

  /// @brief 作ってあるエンティティをメニューの親として設定する
  /// @param defaultButton 選択なしで方向入力があったときに選ぶボタン
  static void SetupMenu(
      entt::registry& registry, entt::entity menuEntity,
      entt::entity defaultButton
  );

  /// @brief 作ってあるエンティティをメニューの子のボタンとして設定する
  /// @note 構造に必要なもの（`Button` / `ScreenPos` / `Hierarchy`）だけを
  /// 付ける。文字などの見た目は画面側で付けること
  static void SetupButton(
      entt::registry& registry, entt::entity buttonEntity,
      entt::entity menuEntity, const ButtonParam& param
  );
};
