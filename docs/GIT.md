# Git / GitHub 運用ルール

## ブランチ戦略

| 作業種別 | 方針 |
|----------|------|
| `enhancement` / `bug` | ブランチを切ってPRを作成 → mainにマージ |
| `chore` | mainに直接コミット |

## ブランチ命名規則

```
feature/<short-description>   # 新機能
fix/<short-description>        # バグ修正
```

例: `feature/player-movement`, `fix/collision-bug`

## コミットメッセージとissueの紐付け

関連するissueは常にコミットメッセージに含める。

```
Add player movement #3              # 参照のみ（issueは開いたまま）
Fix collision detection, close #5   # mainマージ時にissueが自動クローズ
```

`close` の代わりに `fixes` / `resolves` も使用可。

## コミットの修正

GitHub Actions の失敗など、直前のコミットへの軽微な追加・修正は `git commit --amend` を使う。

```bash
git commit --amend --no-edit   # メッセージを変えない場合
git commit --amend             # メッセージも修正する場合
```

ただし **push 済みのコミットを amend した場合は force push が必要**になるため、
main への直接 amend は避け、feature ブランチ上でのみ使用する。

## issueラベル

| ラベル | 用途 |
|--------|------|
| `epic` | 複数のsub-issueをまとめる親チケット |
| `enhancement` | 新機能・改善 |
| `bug` | 不具合修正 |
| `chore` | リファクタリング・環境整備など |
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

## PRのマージ方針

- 通常マージ（Merge commit）を使用する
- PRタイトルは英語・命令形
- マージ後はブランチを削除する

## issueタイトル

- 日本語で書く
