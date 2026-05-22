# Git / GitHub 運用ルール

## ブランチ戦略

| 作業種別 | 方針 |
|----------|------|
| `enhancement` / `bug` / `refactor` | ブランチを切ってPRを作成 → mainにマージ |
| `chore` | mainに直接コミット |

## ブランチ命名規則

```
feature/<short-description>   # 新機能
fix/<short-description>        # バグ修正
refactor/<short-description>   # リファクタリング
```

例: `feature/player-movement`, `fix/collision-bug`, `refactor/vcpkg-entt`

## コミットメッセージとissueの紐付け

関連するissueは常にコミットメッセージに含める。

```
Add player movement #3              # 参照のみ（issueは開いたまま）
Fix collision detection, close #5   # mainマージ時にissueが自動クローズ
```

`close` の代わりに `fixes` / `resolves` も使用可。

## コミットの整理

PRのマージ前に `git rebase -i` で意味のある単位に整理すること。

- 機能追加・バグ修正・リファクタなど論理的に異なる変更は別コミットに分ける
- typo修正・lint警告・ビルドエラーなど軽微な修正は直前の関連コミットにまとめる

```bash
git rebase -i origin/main   # mainからの全コミットを対象に整理
```

整理後のforce-pushは `--force-with-lease` を使うこと。
**mainブランチへのforce-pushは理由を問わず禁止。**

```bash
git push --force-with-lease   # featureブランチへのforce-push
```

## issueラベル

| ラベル | 用途 |
|--------|------|
| `epic` | 複数のsub-issueをまとめる親チケット |
| `enhancement` | 新機能・改善 |
| `bug` | 不具合修正 |
| `refactor` | リファクタリング（PR経由） |
| `chore` | Claude設定・依存更新・環境整備など（直接コミット） |
| `clang-tidy` | clang-tidy 自動レポート（CI が自動で付与） |

## git コマンドの実行形式

ディレクトリを指定して git を実行する場合は、`cd` を使わず `git -C <path>` を使うこと。

```bash
# NG
cd ~/path/to/repo && git status

# OK
git -C ~/path/to/repo status
```

`~/.claude/settings.json` の allow パターンが `git -C *` 形式で定義されているため、
`cd && git` 形式だとルールが適用されない。

## PRの作成

PR を作成する際は必ず `/create-pr` スキルを使うこと。

```
/create-pr               # タイトルを自動生成
/create-pr Add player movement #3  # タイトルを指定
```

このスキルは PR 作成後に自動で `pr-reviewer` エージェントを起動し、
GitHub の PR ページにレビューコメントを投稿する。

## PRのマージ方針

- 通常マージ（Merge commit）を使用する
- PRタイトルは日本語で書く
- マージ後はブランチを削除する

## issueタイトル

- 日本語で書く
