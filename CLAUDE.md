# CLAUDE.md

## プロジェクト概要

「沼に焚べ」— 疑似3D視点のベルトアクションゲーム。
C++ / Siv3D v0.6.16 / Visual Studio 2022

## 開発の目的

1. ゲームの開発
2. モダンなC++の勉強（新しい書き方・機能を積極的に採用する）
3. バイブコーディングの経験

## ツール使用ポリシー

ファイル操作は Glob（探索）・Grep（検索）・Read（読み込み）・Edit / Write（編集）を優先する。

サブエージェントを起動するときは、このルールをプロンプトにも明記すること。

## ビルド

ビルドは `tools/build.sh` を実行して行う。デバッガを使った実行・調査はユーザーが Visual Studio 2022 で行う。

## テスト

[TEST.md](docs/TEST.md) を参照。

## Git / GitHub 運用

git または GitHub の操作（commit・push・PR・issue 等）を始める前に、
一連の作業の最初に一度 [GIT.md](docs/GIT.md) を読むこと。
