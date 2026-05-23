---
name: run
description: Run the app (tools/run.sh) in the background and monitor output. Use when asked to run, start, or test the app.
---

`./tools/run.sh` をバックグラウンドで起動し、出力を監視する。

## 手順

1. `run_in_background: true` を指定して `./tools/run.sh` を Bash で起動する
2. Monitor ツールでプロセスの出力ストリームを監視する
3. ゲームが終了したら Monitor の出力を確認してユーザーに報告する
   - `WARNING: App exited with non-zero status` が出ていればランタイムエラーあり
   - 例外・クラッシュ情報は `Ash2/App/crash.log` を確認する

## 注意

- `runtime.log` は常に空（Console.open() が stdout/stderr を Win32 コンソールへリダイレクトするため読まない）
- 例外・クラッシュ情報は `Ash2/App/crash.log` に追記される
- ユーザーがゲームを閉じるまでプロセスは終了しない
