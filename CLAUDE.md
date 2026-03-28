# CLAUDE.md

## プロジェクト概要

「沼に焚べ」— 疑似3D視点のベルトアクションゲーム。
C++ / Siv3D v0.6.15 / Visual Studio 2022

## 開発の目的

1. ゲームの開発
2. モダンなC++の勉強（新しい書き方・機能を積極的に採用する）
3. バイブコーディングの経験

## コーディングスタイル

[CODING_STYLE.md](CODING_STYLE.md) を参照。

## ビルド

コンパイルエラーの確認は MSBuild で行う。

```bash
msbuild Ash2/Ash2.sln
```

動作確認・デバッグはユーザーが Visual Studio 2022 で直接実行する。
