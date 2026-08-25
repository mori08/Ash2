#include <Siv3D.hpp>

#include "CrashHandler.hpp"

#include <cstdlib>
#include <iostream>

#include "Debug.hpp"

namespace {

/// @brief 分類に対応するプレイヤー向けの文言
[[nodiscard]] String PlayerMessage(const FatalReason reason) {
  switch (reason) {
    case FatalReason::AssetMissing:
      return U"ゲームのデータを読み込めませんでした。"
             U"再インストールをお試しください。";
    case FatalReason::ConfigInvalid:
      return U"設定を読み込めませんでした。"
             U"設定ファイルの削除で復旧する場合があります。";
    case FatalReason::Unknown:
      break;
  }
  return U"予期しないエラーが発生しました。";
}

/// @brief 分類の記録用の名前
[[nodiscard]] StringView ReasonName(const FatalReason reason) {
  switch (reason) {
    case FatalReason::AssetMissing:
      return U"AssetMissing";
    case FatalReason::ConfigInvalid:
      return U"ConfigInvalid";
    case FatalReason::Unknown:
      break;
  }
  return U"Unknown";
}

}  // namespace

void ExitWithFatal(const FatalError& error) {
  const String record = U"[{}] {}: {}"_fmt(
      DateTime::Now(), ReasonName(error.reason), error.detail
  );
  TextWriter{U"crash.log", OpenMode::Append}.writeln(record);

#ifdef _DEBUG
  APP_LOG(record);
#else
  // detail は開発者向けのため、画面には分類に対応する文言だけを出す
  System::MessageBoxOK(
      U"エラー",
      U"{}\n\n詳細は crash.log に記録されています。"_fmt(
          PlayerMessage(error.reason)
      )
  );
#endif

  ExitImmediately(EXIT_FAILURE);
}

void ExitImmediately(const int32 exitCode) {
  // std::exit() は s3d のスレッドと噛み合って終了しない
  // _Exit() は出力を流さないため、先に明示的に流す
  // TODO(#278): 終了コードを設定する手段がなく _Exit に頼っている
  std::cout.flush();
  std::_Exit(exitCode);
}
