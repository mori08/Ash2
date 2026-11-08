---
name: run
description: Run the app under cdb (tools/run-cdb.sh) in the background and report the result. Use when asked to run, start, or test the app, to check screenshots while it runs, or to capture a screenshot automatically (tools/capture.sh).
---

`./tools/run-cdb.sh` をバックグラウンドで起動し、終了したら結果を報告する。

## 手順

1. `run_in_background: true` を指定して `./tools/run-cdb.sh` を Bash で起動する
2. 終了の通知を待ち、出力の末尾と終了コードを確認してユーザーに報告する
   - 正常終了なら `正常に終了しました` が出力され、終了コードは 0
   - クラッシュなら `Last event: ...` と `ERROR: クラッシュしました。ダンプ: <パス>` が出力され、終了コードは 1。`Last event` の行とダンプのパスを報告する
   - ユーザーが閉じる前にゲームが終了したら、ユーザーに確認する

## 開始セクションの指定

`./tools/run-cdb.sh --section <name>` で `scenario.toml` のセクションから起動する。

- 起動先は既定でタイトル画面。TestMenu へ行くときは `--section TestMenu` を渡す

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

## スクリーンショット

- 保存先は `Ash2/App/screenshots/`。`tools/run-cdb.sh` が起動時に PNG を消す
- 撮影キーは PrintScreen と Shift などの同時押しを案内する。F12 は案内しない
- 「確認して」と言われたら、最新の画像を Read で読む

### 自動確認

「撮影しながら確認する」と頼まれたときだけ、起動と同時に Monitor を `timeout_ms: 1800000` で次のコマンドで動かす。

```bash
dir=Ash2/App/screenshots
last=$(date +%Y%m%d-%H%M%S)
while sleep 1; do
  new=$(ls "$dir" 2>/dev/null | grep '\.png$' | tail -n 1)
  if [[ "$new" > "$last" ]]; then
    echo "$new"
    last=$new
  fi
done
```

- 通知されたファイルを Read で読み、問題点と気づいた点だけを短く伝える
- ゲームの終了を通知されたら TaskStop で止める
- 期限切れでは再開せず、頼まれたときに再開する

### 自動撮影

「画面を撮って確認して」と頼まれたときは、バックグラウンドにせずフォアグラウンドで次を実行する。

```bash
./tools/capture.sh <section> <秒>
```

- `<section>` は `scenario.toml` のセクション名。`init` から撮るときは `init` を渡す
- `<秒>` は開始からの経過秒数。1 回の起動で 1 枚撮影し、ゲームは自動で終了する
- 成功すると保存された PNG のパスを 1 行出力する。Read で読み、問題点と気づいた点だけを短く伝える
- 失敗したら終了コードと出力を報告する。理由は `./tools/read-dump.sh` の `[message]` で確認する
- `0` 秒は最初のフレームを撮る。`ScenarioPhase` は 1 フレーム 1 ステップ進むため、画面が未完成のことがある
- 撮影中、ゲームのウィンドウがデスクトップに一瞬表示される
- ステージは `./tools/capture.sh Stage1 <秒>` で撮る
- それ以外の途中の画面を撮るときは、`scenario.toml` に撮影用のセクションを足す

```toml
[[CapturePlayerAnimation]]
action = "push"
phase = "animation_viewer"
param = "player"
```

## 注意

- ユーザーがゲームを閉じるまでプロセスは終了しない
- ゲームの標準出力は Win32 コンソールへ送られるため、出力ファイルには現れない
- cdb の全出力は `logs/cdb.log` に残る
