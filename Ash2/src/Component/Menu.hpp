#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief メニューの親エンティティに付ける。子はボタンだけに限る
struct Menu {
  /// 選択なしの状態で方向入力があったときに選ぶボタン（子のいずれか）
  entt::entity defaultButton = entt::null;
};

/// @brief メニューの子のボタン
///
/// `ScreenPos::pos` を中心とする矩形 `size` が当たり範囲になる。
struct Button {
  /// 当たり範囲の大きさ（px）
  SizeF size{};
  /// 各方向の移動先。移動先がない方向は entt::null。同じメニューの子に限る
  entt::entity up = entt::null;
  entt::entity down = entt::null;
  entt::entity left = entt::null;
  entt::entity right = entt::null;
};

/// @brief 選択中のボタンに付けるタグ。選択状態はここに一本化する
/// @note 1メニューにつき高々1つの子にだけ付く
struct Focused {};
