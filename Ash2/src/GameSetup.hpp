#pragma once
#include <Siv3D.hpp>

#include <entt/entt.hpp>

/// @brief Siv3D 側の初期設定を行う
///
/// アセット・UI フォントの登録、シーンの設定、ウィンドウプリセットの適用を
/// まとめる。
/// @throw Error アセット・UI フォントの登録やウィンドウの変更に失敗したとき
void InitializeEngine();

/// @brief registry のコンテキストを初期化する
/// @throw Error 設定の読み込みに失敗したとき、
/// またはシナリオに init セクションがないとき
void InitializeRegistry(entt::registry& registry);

/// @brief 設定を読み込んで registry のコンテキストに載せる
///
/// 起動時の InitializeRegistry と、F5 の再読込（DebugOnly::UpdateConfigReload）
/// の両方から呼ばれる。既に載っている設定は上書きする。
/// ScenarioData は ScenarioPhase が実行中にステップ番号を持つため含めない。
/// @throw Error 設定の読み込みに失敗したとき
void LoadConfigs(entt::registry& registry);
