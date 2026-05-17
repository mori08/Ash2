#pragma once
#include <Siv3D.hpp>

/// @brief デバッグ・リリース共通のアセットリストを返す
/// @details `tools/sync-assets.sh` で生成した `asset/asset_list` を読む。
///          デバッグ時はファイルから、リリース時は埋め込みリソースから読む。
/// @return アセットパスの配列
[[nodiscard]] inline Array<FilePath> GetAssetList() {
#ifdef _DEBUG
  TextReader reader(U"asset/asset_list");
  if (not reader) {
    Logger << U"[Asset] asset/asset_list not found. Run tools/sync-assets.sh.";
    return {};
  }
#else
  TextReader reader(Resource(U"asset/asset_list"));
  if (not reader) {
    throw Error{U"asset/asset_list not embedded. Add it to Resource.rc."};
  }
#endif
  Array<FilePath> list;
  while (const auto line = reader.readLine()) {
    list << line.value();
  }
  return list;
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
