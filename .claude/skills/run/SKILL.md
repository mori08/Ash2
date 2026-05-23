---
name: run
description: Run the app (tools/run.sh) in the background and monitor output. Use when asked to run, start, or test the app.
---

`./tools/run.sh` をバックグラウンドで起動し、出力を監視する。

## 手順

1. `run_in_background: true` を指定して `./tools/run.sh` を Bash で起動する
2. Monitor ツールでプロセスの出力ストリームを監視する
3. ゲームが終了したら `logs/runtime.log` を確認してユーザーに報告する
   - `WARNING: App exited with non-zero status` が出ていればランタイムエラーあり
   - ログが空の場合は正常終了（または #92 未解決によりキャプチャ不可）

## 注意

- ゲームは GUI アプリのため、#92 解決前は `runtime.log` が空になる
- ユーザーがゲームを閉じるまでプロセスは終了しない
