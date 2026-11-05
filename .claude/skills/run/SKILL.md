---
name: run
description: Run the app under cdb (tools/run-cdb.sh) in the background and report the result. Use when asked to run, start, or test the app.
---

`./tools/run-cdb.sh` をバックグラウンドで起動し、終了したら結果を報告する。

## 手順

1. `run_in_background: true` を指定して `./tools/run-cdb.sh` を Bash で起動する
2. 終了の通知を待ち、出力の末尾と終了コードを確認してユーザーに報告する
   - 正常終了なら `正常に終了しました` が出力され、終了コードは 0
   - クラッシュなら `Last event: ...` と `ERROR: クラッシュしました。ダンプ: <パス>` が出力され、終了コードは 1。`Last event` の行とダンプのパスを報告する
   - ユーザーが閉じる前にゲームが終了したら、ユーザーに確認する

## クラッシュの調査

調査を頼まれたら、`./tools/read-dump.sh` で `logs/crash.dmp` の要約を読む。

| 出力 | 内容 |
|------|------|
| `[event]` | 例外コードと種類 |
| `[detail]` | 不正アクセスの読み書きの別とアドレス |
| `[type]` / `[message]` | C++ 例外の型とメッセージ。メッセージ内の改行とタブは `.` になる |
| `[assert]` / `[assert_at]` | assert の式と場所 |
| `[stack]` | `Main` までのスタック。先頭の番号は16進数 |

- PDB が一致しないエラーが出たら、再現し直すようユーザーに伝える
- 掘り下げるときは cdb コマンドを引数で渡す
  - 指定のフレームのローカル変数: `.frame <番号>; dv /t`
  - 上から6フレームのローカル変数: `dx -r2 @$curstack.Frames.Take(6).Select(f => f.LocalVariables)`
  - 引数は `kP` ではなく `dx` で見る

## 注意

- ユーザーがゲームを閉じるまでプロセスは終了しない
- ゲームの標準出力は Win32 コンソールへ送られるため、出力ファイルには現れない
- cdb の全出力は `logs/cdb.log` に残る
