---
paths:
  - "Ash2/src/**/*.cpp"
  - "Ash2/src/**/*.hpp"
  - "Ash2/tests/**/*.cpp"
---

# コーディングスタイル

## フォーマット自動適用

clang-format はバージョン **19 系**を使用する。

`.cpp` / `.hpp` ファイルを編集したら、必ず以下のコマンドでフォーマットをかける：

```bash
clang-format -i <ファイルパス>
# PATH に無い場合: find "/c/Program Files/Microsoft Visual Studio" -name "clang-format.exe"
```

## ファイルの追加

新しい `.cpp` / `.hpp` ファイルを追加するときは、`Ash2.vcxproj` と `Ash2.vcxproj.filters` の編集も必要。
