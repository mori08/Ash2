---
name: resolve-issue
description: |
  Issue番号を指定して呼び出す（例: "resolve-issue #42" や "#42を解決して"）。
  Issueの調査・計画・実装・PRまでの一連のフローを起動する。
  Issueの実装に着手するときは必ずこのSkillを使うこと。
---

# resolve-issue

Issueを受け取り、全フェーズをTaskとして登録した上で順番に実行する起点となるフロー。

## 起動時の処理

`$ARGUMENTS` からIssue番号を取得する。

TaskList で未完了タスクを確認する（未完了タスクがある場合は該当フェーズから再開する）。

### 新規の場合

TaskCreate で以下の全タスクを登録する。

```
[ ] Phase 1: plan-issue      - 調査・実装計画の策定・ブランチ作成
[ ] Phase 2: implement-issue - 実装ループ・確認・ビジュアルチェック
[ ] Phase 3: commit-push     - コミットとpush
[ ] Phase 4: create-pr       - PRの作成とレビュー
[ ] Phase 5: cleanup         - マージ後の片付け（マージ報告待ち）
```

## フェーズの実行

Phase 1 以降は対応するSkillを順番に呼び出す。
各Skillの完了後、TaskUpdate で `completed` に更新してから次のフェーズへ進む。

**ユーザーの承認待ちが発生するフェーズ（Phase 1・2・5）では、
承認を得てから次のSkillを呼び出すこと。**
