#pragma once
#include <Siv3D.hpp>

#include "FatalError.hpp"

/// @brief 続行できない失敗を記録・表示して終了する
/// @note Main() の catch からのみ呼ぶ。終了を決めた側は FatalError を投げる
[[noreturn]] void ExitWithFatal(const FatalError& error);
