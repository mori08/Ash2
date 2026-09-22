#pragma once

/// @brief 撃破され行動不能であることを示すタグコンポーネント
///
/// `PlayerMotion::Variant` が `PlayerMotion::Dead` へ遷移するのと同時に
/// 付与される。索敵など、撃破後は働かせたくない処理から
/// `entt::exclude<Dead>` で除外する。
struct Dead {};
