#pragma once
#include <Siv3D.hpp>

/// @brief 続行できない失敗の分類
enum class FatalReason : uint8 {
  /// アセットの欠落・破損
  AssetMissing,
  /// 設定ファイルの不正
  ConfigInvalid,
  /// 分類のない失敗
  Unknown,
};

/// @brief 続行できない失敗
struct FatalError {
  /// 画面に出す文言はこの分類から決まる
  FatalReason reason;
  /// 開発者向けの詳細。crash.log にのみ出る
  String detail;
};
