#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

#include "Input/InputState.hpp"

/// @brief メニューの選択の規則を適用するシステム
///
/// 選択状態は子のボタンの `Focused` に一本化し、そのフレームに起きた操作だけで
/// 動かす。見た目は扱わない。
class MenuSystem {
 public:
  /// @brief
  /// メニューの子のボタンに入力を適用し、このフレームに決定されたボタンを返す
  ///
  /// 次の順で適用する。
  /// 1. カーソルが動いた: カーソルの下のボタンを選ぶ。なければ選択なし
  /// 2. 方向入力: 選択があれば移動先へ、なければ既定のボタンを選ぶだけ
  /// 3. 決定: 選択があればそのボタンを返す
  /// 4. クリック: `pointerPos` の下のボタンを選んで返す。なければ none
  /// @return 決定されたボタン。なければ none
  /// @note `menuEntity` は `Menu` を持ち、子がすべて `Button` + `DrawPos`
  /// を持つこと。`defaultButton` と移動先は同じメニューの子であること
  /// @throw Error `menuEntity` が `Menu` を持たないとき。選択の移動先
  /// （`defaultButton`・方向の移動先）がメニューの子でないとき。カーソルの
  /// 当たり判定で辿った子が `Button` と `DrawPos` を持たないとき
  [[nodiscard]] static Optional<entt::entity> Update(
      entt::registry& registry, const InputState& input, entt::entity menuEntity
  );
};
