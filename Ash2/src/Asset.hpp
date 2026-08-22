#pragma once
#include <Siv3D.hpp>

#include <expected>

/// @brief デバッグ・リリース共通のアセットパスを返す
/// @return デバッグ時は FilePath、リリース時は Resource パスを返す
[[nodiscard]] inline FilePath AssetPath(StringView path) {
#ifdef _DEBUG
  return FilePath{path};
#else
  return Resource(path);
#endif
}

/// @brief デバッグ・リリース共通のアセットリストを返す
///
/// `tools/sync-assets.sh` で生成した `assets/asset_list` を読む。
/// @return 失敗時は開けなかったパスを含むメッセージ
[[nodiscard]] inline std::expected<Array<FilePath>, String> GetAssetList() {
  const FilePath path = AssetPath(U"assets/asset_list");
  TextReader reader(path);
  if (not reader) {
    return std::unexpected{
        U"GetAssetList: {} を開けません / tools/sync-assets.sh を実行する"_fmt(
            path
        )
    };
  }
  Array<FilePath> list;
  while (const auto line = reader.readLine()) {
    list << line.value();
  }
  return list;
}

/// @brief アセット配下の TOML を開く
/// @return 開けない場合はパスを含むメッセージ
[[nodiscard]] inline std::expected<TOMLReader, String> OpenToml(
    StringView path
) {
  TOMLReader toml{AssetPath(path)};
  if (not toml.isOpen()) {
    return std::unexpected{U"OpenToml: {} を読み込めません"_fmt(path)};
  }
  return toml;
}

/// @brief アセットをアセットシステムに登録する
///
/// `.png` を TextureAsset、`.mp3` を AudioAsset として登録する。
/// キーはファイルの相対パス（例: `assets/images/player.png`）。
/// @return 失敗時は登録できなかったパスを含むメッセージ
[[nodiscard]] inline std::expected<void, String> RegisterAssets() {
  auto list = GetAssetList();
  if (!list) {
    return std::unexpected{std::move(list).error()};
  }
  for (const auto& path : *list) {
    const auto ext = FileSystem::Extension(path);
    if (ext == U"png") {
      if (!TextureAsset::Register(path, AssetPath(path))) {
        return std::unexpected{
            U"RegisterAssets: {} を登録できません"_fmt(path)
        };
      }
    } else if (ext == U"mp3") {
      if (!AudioAsset::Register(path, AssetPath(path))) {
        return std::unexpected{
            U"RegisterAssets: {} を登録できません"_fmt(path)
        };
      }
    }
  }
  return {};
}
