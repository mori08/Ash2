---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、**ビルド前に** `Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。

## 静的解析・フォーマット

clang-format / clang-tidy はバージョン **19 系**を使用する。

ファイルを編集したら、必ず以下のコマンドを実行する（`stdafx.cpp` は除く）：

```bash
# .cpp を編集した場合（format → tidy の順に実行）
./tools/run-format.sh <ファイルパス>
./tools/run-tidy.sh <ファイルパス>

# .hpp のみ編集した場合（format のみ）
./tools/run-format.sh <ファイルパス>
```

clang-tidy は時間がかかるため `run_in_background: true` でバックグラウンド実行し、
その間にドキュメント更新・vcxproj 編集など tidy 結果に依存しない作業を進める。

完了通知の status が `completed`（exit code 0）なら出力ファイルは読まない。
`failed`（exit code 1）のときだけ出力ファイルを Read して原因を確認する。

警告が出た場合はすべて修正し、修正後に再チェックしてからコミットすること：

```bash
./tools/run-tidy.sh <ファイルパス>
```

**備考：**
- `.hpp` の変更を tidy で検証したい場合はインクルードしている `.cpp` を指定すること
- `Ash2/.tidy/cpuid.h` は DirectXMath 互換のスタブ（削除しないこと）
- clang-format / clang-tidy が PATH にない場合は追加する

## ビルド

lint が通ったら `tools/build.sh` でビルドする。リポジトリルートから実行すること。

```bash
./tools/build.sh                    # 通常ビルド（sync-assets → MSBuild）
./tools/build.sh --no-sync-assets   # アセット未変更時の高速リビルド
```

### 主要オプション（MSBuild）

| オプション | 値 | 説明 |
|---|---|---|
| `-p:Configuration` | `Debug` / `Release` | ビルド構成（デフォルト: Debug） |
| `-p:Platform` | `x64` | ターゲットプラットフォーム |
| `-m` | — | 並列ビルド（CPU数に合わせる） |
| `-v` | `minimal` / `normal` / `detailed` / `diag` | ターミナル出力の詳細度 |
| `-t` | `Build` / `Clean` / `Rebuild` | ターゲット指定 |

### ビルドログ

- **ターミナル**（`-v:minimal`）：エラーと警告のみ。Claude Code が直接読み取る
- **`logs/build.log`**（`-v:detailed`）：詳細ログ。コンパイルエラーの調査に使用

完了通知の status が `completed`（exit code 0）なら出力ファイルは読まない。
`failed`（exit code 1）のときだけ出力ファイルを Read して原因を確認する。

### 自動テスト

ビルド成功後、必ず `tools/run-tests.sh` を実行してテストが通ることを確認する。

```bash
./tools/run-tests.sh
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
  [[nodiscard]] Vec2 toScreen() const;
};

/// @brief 描画順の比較関数（奥から手前の順）
/// @param a 比較対象A
/// @param b 比較対象B
/// @return a が b より奥にある場合 true
inline bool DrawOrderLess(const WorldPos& a, const WorldPos& b);
```

## ARCHITECTURE.md の更新

`docs/ARCHITECTURE.md` はプロジェクトの構造をまとめたドキュメント。

以下の変更を行った場合、実装後・返答前に必ず ARCHITECTURE.md を読み、
現状と乖離があれば更新すること。

- 新しいクラス・構造体の追加
- ディレクトリ構成の変更
- アーキテクチャ方針の変更（フェーズ管理・ECS の使い方など）
- 座標系・ゲームループの変更

### ファイルへのリンク

クラス・システム・フェーズなどを一覧するテーブルでは、名前をリポジトリルートからの相対パスリンクにすること。
リンク形式は `[ClassName](../Ash2/src/Path/To/File.hpp)` （`docs/` からの相対パス）。

### 200行上限のルール

超過しそうな場合は以下の優先順位で削る：
1. コード例（コードを見ればわかる）
2. 実装の詳細（How より Why を残す）
3. 現在使われていない設計の説明

削ってはいけないもの：レイヤー構成・座標系・主要な制約・設計の意図
