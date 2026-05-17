#pragma once
#include <Siv3D.hpp>

/// @brief デバッグ・リリース共通のアセットリストを返す
/// @details デバッグ時は `asset/` を再帰スキャンし `asset/asset_list`
/// に書き出す。
///          リリース時は埋め込みリソースの `asset/asset_list`
///          を読み、相対パスとして返す。
/// @return アセットパスの配列
[[nodiscard]] inline Array<FilePath> GetAssetList() {
#ifdef _DEBUG
  Array<FilePath> list;
  for (const auto& content :
       FileSystem::DirectoryContents(U"asset", Recursive::Yes)) {
    if (FileSystem::IsDirectory(content)) {
      continue;
    }
    const auto relativePath = FileSystem::RelativePath(content);
    if (relativePath == U"asset/asset_list") {
      continue;
    }
    list << relativePath;
  }
  TextWriter writer(U"asset/asset_list");
  if (not writer) {
    Logger << U"[Asset] Failed to write asset/asset_list. Create the asset/ "
              U"directory.";
    return list;
  }
  for (const auto& path : list) {
    writer.writeln(path);
  }
  return list;
#else
  TextReader reader(Resource(U"asset/asset_list"));
  if (not reader) {
    throw Error{U"asset/asset_list not embedded. Add it to Resource.rc."};
  }
  Array<FilePath> list;
  while (const auto line = reader.readLine()) {
    list << line.value();
  }
  return list;
#endif
}

/// @brief デバッグ・リリース共通のアセットパスを返す
/// @param path アセットの相対パス（例: `asset/image/player.png`）
/// @return デバッグ時は FilePath、リリース時は Resource パス
[[nodiscard]] inline FilePath AssetPath(StringView path) {
#ifdef _DEBUG
  return FilePath{path};
#else
  return Resource(path);
#endif
}

/// @brief アセットをアセットシステムに登録する
/// @details `.png` を TextureAsset、`.mp3` を AudioAsset として登録する。
///          キーはファイルの相対パス（例: `asset/image/player.png`）。
inline void RegisterAssets() {
  for (const auto& path : GetAssetList()) {
    const auto ext = FileSystem::Extension(path);
    if (ext == U"png") {
      TextureAsset::Register(path, AssetPath(path));
    } else if (ext == U"mp3") {
      AudioAsset::Register(path, AssetPath(path));
    }
  }
}
