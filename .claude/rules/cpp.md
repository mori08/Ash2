---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

## 静的解析（clang-tidy）

clang-tidy はバージョン **19 系**を使用する。

`.cpp` ファイルを編集したら、必ず以下のコマンドで静的解析をかける（`stdafx.cpp` は除く）：

```bash
REPO=$(git rev-parse --show-toplevel)
PROJECT_WIN=$(cygpath -w "$REPO/Ash2")
SIV3D_WIN=$(cygpath -w "$SIV3D_0_6_16")
TIDY_WIN=$(cygpath -w "$REPO/Ash2/.tidy")
CLANG_TIDY=$(find "/c/Program Files/Microsoft Visual Studio" -path "*/Llvm/x64/bin/clang-tidy.exe" 2>/dev/null | head -1)
TARGET_WIN=$(cygpath -w <ファイルパス>)

MSYS_NO_PATHCONV=1 MSYS2_ARG_CONV_EXCL="*" \
"$CLANG_TIDY" \
  "--header-filter=.*Ash2[\\/](src|tests)[\\/].*" \
  "$TARGET_WIN" \
  -- \
  --driver-mode=cl /std:c++latest /Zc:__cplusplus /utf-8 \
  "/FI${PROJECT_WIN}\\src\\stdafx.h" \
  -D_DEBUG -D_WINDOWS \
  -D_ENABLE_EXTENDED_ALIGNED_STORAGE \
  -D_SILENCE_CXX20_CISO646_REMOVED_WARNING \
  -D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS \
  -D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS \
  -DUSE_TEST \
  "-I${TIDY_WIN}" "-I${PROJECT_WIN}\\src" "-I${PROJECT_WIN}" \
  -imsvc "${SIV3D_WIN}\\include" \
  -imsvc "${SIV3D_WIN}\\include\\ThirdParty"
```

警告が出た場合はすべて修正してから返答すること。
修正後は必ず clang-format もかけること（下記参照）。

**備考：**
- `SIV3D_0_6_16` は Siv3D SDK インストール時に設定される環境変数
- `-imsvc` で Siv3D をシステムヘッダー扱いにし、DirectXMath 由来のエラーを抑制
- `Ash2/.tidy/cpuid.h` は DirectXMath 互換のスタブ（削除しないこと）
- `.hpp` 単体の編集時はインクルードしている `.cpp` に対して実行すること

## フォーマット自動適用

clang-format はバージョン **19 系**を使用する。

`.cpp` / `.hpp` ファイルを編集したら、必ず以下のコマンドでフォーマットをかける：

```bash
clang-format -i <ファイルパス>
# PATH に無い場合: find "/c/Program Files/Microsoft Visual Studio" -name "clang-format.exe"
```

## コメント（.hpp）

`.hpp` の関数・構造体・メンバ変数には `///` で Doxygen コメントを書く。言語は日本語。

- 関数: `@brief`（必須）、`@param` / `@return`（引数・戻り値がある場合は必須）
- 構造体: `@brief` で一行説明
- メンバ変数: `///` で一行説明（前置）
- 自明なもの（getter 等）は省略可

```cpp
/// @brief ワールド座標
struct WorldPos {
  /// 横位置
  double w = 0.0;
  /// 高さ（地面 = 0、上方向が正）
  double h = 0.0;

  /// @brief ワールド座標を画面座標に変換する
  /// @return 画面座標（右方向・下方向が正）
  [[nodiscard]] Vec2 ToScreen() const;
};

/// @brief 描画順の比較関数（奥から手前の順）
/// @param a 比較対象A
/// @param b 比較対象B
/// @return a が b より奥にある場合 true
inline bool DrawOrderLess(const WorldPos& a, const WorldPos& b);
```

## 命名規則

静的メンバ関数・通常関数（free function）は PascalCase（大文字始まり）で書く。

```cpp
// OK
static PlayerConfig FromToml(const TOMLValue& toml);
bool DrawOrderLess(const WorldPos& a, const WorldPos& b);

// NG
static PlayerConfig fromToml(const TOMLValue& toml);
```

## マジックナンバー

`readability-magic-numbers` により、以下の値のみリテラルとして直接使用できる。

| 種別 | 許可値 |
|------|--------|
| 整数 | `0`, `1`, `-1`, `2` |
| 浮動小数点 | `0.0`, `1.0`, `0.5` |

それ以外はマジックナンバーとみなされエラーになる。`constexpr` 定数として定義すること。

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、`Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。

## ARCHITECTURE.md の更新

`docs/ARCHITECTURE.md` はプロジェクトの構造をまとめたドキュメント。

以下の変更を行った場合、実装後・返答前に必ず ARCHITECTURE.md を読み、
現状と乖離があれば更新すること。

- 新しいクラス・構造体の追加
- ディレクトリ構成の変更
- アーキテクチャ方針の変更（フェーズ管理・ECS の使い方など）
- 座標系・ゲームループの変更

### 200行上限のルール

超過しそうな場合は以下の優先順位で削る：
1. コード例（コードを見ればわかる）
2. 実装の詳細（How より Why を残す）
3. 現在使われていない設計の説明

削ってはいけないもの：レイヤー構成・座標系・主要な制約・設計の意図
