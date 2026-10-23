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

調査を頼まれたら、`logs/crash.dmp` を cdb で開いて読む。

```bash
cdbX64.exe -z "<リポジトリ>/logs/crash.dmp" \
  -y "<リポジトリ>/Ash2/Intermediate/Ash2/Debug;srv*" \
  -c "ld Ash2_debug_; lm m Ash2_debug_; .lines -e; k 10; q" \
  2>&1 | iconv -f CP932 -t UTF-8
```

- パスは `C:/...` 形式（`git rev-parse --show-toplevel` の形式）で渡す
- `-y` を省略しない
- `lm` の結果が `(private pdb symbols)` でなければ読まない。ゲームを再ビルドしたためなので、再現し直すようユーザーに伝える
- 掘り下げるときは `-c` の中身を変える
  - C++ 例外（`eh`）のメッセージ: `.frame 1; dx ((s3d::Error*)pExceptionObject)->m_what.m_string`
  - 上から6フレームのローカル変数: `dx -r2 @$curstack.Frames.Take(6).Select(f => f.LocalVariables)`
  - 引数は `kP` ではなく `dx` で見る

## 注意

- ユーザーがゲームを閉じるまでプロセスは終了しない
- ゲームの標準出力は Win32 コンソールへ送られるため、出力ファイルには現れない。出力ファイルに現れるのは cdb の出力のみ
