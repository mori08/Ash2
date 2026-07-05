# 開発環境セットアップ

新しい端末でこのプロジェクトの開発を始めるための手順。

- 対応 OS: **Windows 10 / 11**（Windows のみ対応。動作確認は 11 のみ）
- コマンドは特記がない限り **Git Bash** で実行する

## 必須ツール

| ツール | 用途 | 入手方法 |
|--------|------|----------|
| Visual Studio 2022 17.14 以降（C++ デスクトップ開発ワークロード） | ビルド・デバッグ | [公式サイト](https://visualstudio.microsoft.com/) |
| Git for Windows | バージョン管理・`tools/*.sh` の実行環境（Git Bash） | `winget install Git.Git` |
| Siv3D v0.6.16 | ゲームエンジン SDK | [公式サイト](https://siv3d.github.io/ja-jp/)のインストーラ |
| vcpkg | C++ パッケージ管理（entt を manifest モードで取得） | 下記参照 |
| Python 3 | `tools/sync-assets.sh` が使用（ビルド時に自動実行） | `winget install Python.Python.3.12` |
| clang-format / clang-tidy | 整形・静的解析 | Visual Studio に同梱（下記参照） |
| GitHub CLI（gh） | PR・issue 操作 | `winget install GitHub.cli` |
| Claude Code | 開発フローの中心（`.claude/` のスキル・エージェントを実行） | 下記参照 |

## セットアップ手順

### 1. Visual Studio 2022

「C++ によるデスクトップ開発」ワークロードを含めてインストールする。
**17.14 以降**が必要（Siv3D v0.6.16 の要件。古いとリンクエラーになる）。
`tools/build.sh` は vswhere 経由で MSBuild を自動検出する。

### 2. Siv3D v0.6.16

[公式インストーラ](https://siv3d.github.io/ja-jp/)でインストールする。
インストーラが環境変数 `SIV3D_0_6_16` を設定する。新しいターミナルを開いて確認する:

```bash
echo "$SIV3D_0_6_16"
```

別バージョンの Siv3D では代用できない（複数バージョンの共存は可能）。

### 3. vcpkg

```bash
git clone https://github.com/microsoft/vcpkg /c/vcpkg
/c/vcpkg/bootstrap-vcpkg.bat
/c/vcpkg/vcpkg integrate install
```

依存パッケージ（entt）は `Ash2/vcpkg.json` の manifest モードにより
初回ビルド時に自動で取得される。手動での `vcpkg install` は不要。

### 4. Python 3

`tools/sync-assets.sh`（`tools/build.sh` から自動実行）が使用する。

```bash
winget install Python.Python.3.12
```

[公式インストーラ](https://www.python.org/downloads/)でも可。
新しいターミナルで `python --version` を確認する。

### 5. clang-format / clang-tidy

Visual Studio の C++ ワークロードに同梱されているものを使う。
`~/.bashrc` に環境変数を設定する（パスはエディションに合わせて読み替える）:

```bash
export CLANG_FORMAT="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin/clang-format.exe"
export CLANG_TIDY="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin/clang-tidy.exe"
```

### 6. GitHub CLI

```bash
winget install GitHub.cli
gh auth login
```

PR・issue 操作（`/create-pr` スキルや CI 連携）で使用する。

### 7. Claude Code

このプロジェクトは Issue の解決から PR 作成までを Claude Code のスキル
（`/resolve-issue`・`/run`・`/create-pr` など、`.claude/` 配下）で回す前提になっている。

[公式ドキュメント](https://code.claude.com/docs)を参照してインストールし、
リポジトリのルートで `claude` を起動してログインする。

## 動作確認

セットアップ後、以下が順に通れば完了。

```bash
./tools/build.sh        # アセット同期 → MSBuild（初回は vcpkg が entt を取得）
./tools/run-tests.sh    # Catch2 テスト実行
./tools/run.sh          # ゲーム起動
```

整形・静的解析の確認:

```bash
bash tools/run-format.sh Ash2/src/Main.cpp
bash tools/run-tidy.sh Ash2/src/Main.cpp
```

注意: `run-tidy.sh` は初回ビルドで生成される `Ash2/vcpkg_installed/` の
ヘッダを参照するため、ビルドを先に通しておくこと。

## 関連ドキュメント

- [CLAUDE.md](../CLAUDE.md) — 開発フロー全般
- [TEST.md](TEST.md) — テストの書き方・実行方法
- [GIT.md](GIT.md) — Git / GitHub 運用ルール
- [ARCHITECTURE.md](ARCHITECTURE.md) — 設計
