---
name: run
description: Run the app (tools/run.sh) in the background and monitor output. Use when asked to run, start, or test the app.
---

`./tools/run.sh` をバックグラウンドで起動し、出力を監視する。

## 手順

1. `run_in_background: true` を指定して `./tools/run.sh` を Bash で起動する
2. Monitor ツールでプロセスの出力ストリームを監視する
3. ゲームが終了したら Monitor の出力を確認してユーザーに報告する
   - 例外で終了した場合も終了コードは 0 になり、出力からは正常終了と見分けられない
   - ユーザーが閉じる前にゲームが終了したら、ユーザーに確認する
   - クラッシュなら `ERROR: アプリが異常終了しました` が出力される

## 注意

- ユーザーがゲームを閉じるまでプロセスは終了しない
- ゲームの標準出力は Win32 コンソールへ送られるため、Monitor には現れない
