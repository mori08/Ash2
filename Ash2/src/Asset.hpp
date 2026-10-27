#pragma once
#include <Siv3D.hpp>

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
/// @throw Error アセットリストを開けないとき
/// （`tools/sync-assets.sh` を実行していない可能性がある）
[[nodiscard]] inline Array<FilePath> GetAssetList() {
  TextReader reader(AssetPath(U"assets/asset_list"));
  if (not reader) {
    throw Error{U"アセットリストを開けない"};
  }
  Array<FilePath> list;
  while (const auto line = reader.readLine()) {
    list << line.value();
  }
  return list;
}

/// @brief アセット配下の TOML を開く
/// @throw Error TOML ファイルを開けないとき
[[nodiscard]] inline TOMLReader OpenToml(StringView path) {
  TOMLReader toml{AssetPath(path)};
  if (not toml.isOpen()) {
    throw Error{U"TOML ファイルを開けない"};
  }
  return toml;
}

/// @brief アセットをアセットシステムに登録する
///
/// `.png` を TextureAsset、`.mp3` を AudioAsset として登録する。
/// キーはファイルの相対パス（例: `assets/images/player.png`）。
/// @throw Error アセットリストを開けない、または登録に失敗したとき
inline void RegisterAssets() {
  for (const auto& path : GetAssetList()) {
    const auto ext = FileSystem::Extension(path);
    if (ext == U"png") {
      if (!TextureAsset::Register(path, AssetPath(path))) {
        throw Error{U"テクスチャをアセットに登録できない"};
      }
    } else if (ext == U"mp3") {
      if (!AudioAsset::Register(path, AssetPath(path))) {
        throw Error{U"音声をアセットに登録できない"};
      }
    }
  }
}
