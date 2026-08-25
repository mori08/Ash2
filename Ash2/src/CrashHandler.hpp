#pragma once
#include <Siv3D.hpp>

#include "FatalError.hpp"

/// @brief 続行できない失敗を記録・表示して終了する
/// @note Main.cpp の catch からのみ呼ぶ。終了を決めた側は FatalError を投げる
[[noreturn]] void ExitWithFatal(const FatalError& error);

/// @brief 出力を流してからプロセスを即時終了する
/// @note デストラクタ・atexit は実行されない。
/// 終了処理を1本に保つため、ExitWithFatal とテスト実行後の終了からのみ呼ぶ
[[noreturn]] void ExitImmediately(int32 exitCode);
