# CLAUDE.md

## プロジェクト概要

「沼に焚べ」— 疑似3D視点のベルトアクションゲーム。
C++ / Siv3D v0.6.16 / Visual Studio 2022

## 開発の目的

1. ゲームの開発
2. モダンなC++の勉強（新しい書き方・機能を積極的に採用する）
3. バイブコーディングの経験

## ビルド

ビルドは `tools/build.sh` を実行して行う。デバッガを使った調査はユーザーが Visual Studio 2022 で行う。

## 実行

アプリを起動するには `/run` スキルを使う。

## テスト

[TEST.md](docs/TEST.md) を参照。

## 設計ドキュメント

- 設計意図・レイヤー構成・座標系: [ARCHITECTURE.md](docs/ARCHITECTURE.md)
- コンポーネント・システム・フェーズ等の部品一覧: [REFERENCE.md](docs/REFERENCE.md)
- ゲームデザイン仕様: [docs/game_design/](docs/game_design/)
- 開発環境の構築: [SETUP.md](docs/SETUP.md)

## Git / GitHub 運用

git または GitHub の操作（commit・push・PR・issue 等）を始める前に、
一連の作業の最初に一度 [GIT.md](docs/GIT.md) を読むこと。
