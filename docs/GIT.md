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

## issueラベル

| ラベル | 用途 |
|--------|------|
| `epic` | 複数のsub-issueをまとめる親チケット |
| `enhancement` | 新機能・改善 |
| `bug` | 不具合修正 |
| `chore` | リファクタリング・環境整備など |

## PRのマージ方針

- 通常マージ（Merge commit）を使用する
- PRタイトルは英語・命令形
- マージ後はブランチを削除する
