---
name: resolve-issue
description: |
  Issue番号を指定して呼び出す（例: "resolve-issue #42" や "#42を解決して"）。
  Issueの調査・計画・実装・PRまでの一連のフローを起動する。
  Issueの実装に着手するときは必ずこのSkillを使うこと。
---

# resolve-issue

Issueを受け取り、全フェーズを順番に実行する起点となるフロー。

## 起動時の処理

`$ARGUMENTS` からIssue番号を取得する。

Issue のラベルを取得する。ラベルが `chore` の場合は「chore ラベルの Issue は計画不要のため、resolve-issue では対応しません」と伝えてスキルを終了する。

Issue のタイトルをユーザーに提示し、対応する Issue に間違いないか確認を取る。
確認が取れた場合のみ以降の手順に進む。

## フェーズの実行

以下の Skill を順番に呼び出す。

- plan-issue
- implement-issue
- commit-push
- create-pr
- cleanup
