# 沼に焚べ

[![build-test](https://github.com/mori08/Ash2/actions/workflows/build-test.yml/badge.svg)](https://github.com/mori08/Ash2/actions/workflows/build-test.yml)
[![clang-format](https://github.com/mori08/Ash2/actions/workflows/clang-format.yml/badge.svg)](https://github.com/mori08/Ash2/actions/workflows/clang-format.yml)
[![clang-tidy](https://github.com/mori08/Ash2/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/mori08/Ash2/actions/workflows/clang-tidy.yml)
[![CodeQL](https://github.com/mori08/Ash2/actions/workflows/codeql.yml/badge.svg)](https://github.com/mori08/Ash2/actions/workflows/codeql.yml)

疑似3D視点のベルトアクションゲーム。奥行きを持つ2D表現でのアクションを目指した作品。

## 動作環境

- OS: Windows 10 / 11
- [Siv3D](https://siv3d.github.io/) v0.6.16

ビルドに必要なツールとセットアップ手順は [docs/SETUP.md](docs/SETUP.md) を参照。
設計と技術構成は [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) を参照。

## ライセンス

### ソースコード

本リポジトリのソースコードは [MIT License](LICENSE) で公開しています。
著作権表示を保持すれば、利用・改変・再配布は自由です。

### 著作権を留保するもの

以下のファイルは、本リポジトリの [LICENSE](LICENSE) の対象外です（© 2026 mori08）。

- [`Ash2/App/assets/`](Ash2/App/assets/) 配下（画像・音声・テキスト・設定データ等）
- [`ideas/`](ideas/) 配下（初期案・シナリオ原文）

**許可する利用**

- clone してビルド・実行し、ゲームをプレイすること
- 学習目的での閲覧・参照
- ゲーム実況・配信・スクリーンショット・動画サムネイル等での利用（収益化を含む）

**禁止する利用**

- これらのファイルを抽出し、他の作品や用途に再利用・再配布すること
- これらを含んだ状態でのビルド物の再配布・再公開

### サードパーティー

リポジトリに含まれる以下のファイルは、本リポジトリの [LICENSE](LICENSE) の対象外です。
それぞれの提供元のライセンスに従います。

- [`Ash2/App/engine/`](Ash2/App/engine/) 配下 — [Siv3D](https://siv3d.github.io/) v0.6.16 同梱のシェーダ・フォント等（[MIT License](https://github.com/Siv3D/OpenSiv3D/blob/main/LICENSE)）。フォントは [`Ash2/App/engine/font/`](Ash2/App/engine/font/) 配下の各フォルダ内にある LICENSE ファイルを参照
- [`Ash2/App/dll/soundtouch/`](Ash2/App/dll/soundtouch/) — SoundTouch、LGPL v2.1（[COPYING.TXT](Ash2/App/dll/soundtouch/COPYING.TXT)）。動的リンク・無改変
- `Ash2/App/icon.ico` — Siv3D プロジェクトテンプレート付属のアイコン（MIT License）

<!--
  未対応:
  - assets/sounds/ 追加時に「AI を利用していない箇所」へ音声素材の行を足す
  - ideas/scenario/ 作成時:
    - ideas/README.md へ行を足す
    - 「AI を利用していない箇所」のシナリオ原文にパスを足す
-->

## AI の利用について

### AI を利用している箇所

- ソースコードの記述
- ドキュメントの記述
- 設定ファイルの記述
- シナリオテキストの校正・推敲、および多言語への翻訳

いずれも、方針の決定・設計判断・出力のレビューは作者が行っています。

### AI を利用していない箇所

- 画像素材（[`Ash2/App/assets/images/`](Ash2/App/assets/images/)）— EDGE を使用
- 手書きの初期案（[`ideas/sketches/`](ideas/sketches/)）— Concepts を使用
- シナリオ原文
