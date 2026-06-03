# テスト

## 方針

OpenSiv3D に同梱されている **Catch2** を使用する。
テストは Siv3D 環境内で動作するため、すべての Siv3D 型をそのままテスト可能。

## ファイル構成

```
Ash2/
├── src/        ← ゲームのソースコード
└── tests/      ← テストコード（*Test*.cpp）
```

## テストの実行

### テストのみ実行（bash からキャプチャ可能）

```bash
./tools/run-tests.sh
```

テスト完了後にゲームは起動しない。
Catch2 の出力はターミナルに直接表示される。

### ゲームを起動する（通常の開発フロー）

```bash
./tools/run.sh
```

テストは実行されない。ゲームのみ起動する。

## テストの書き方

`tests/` に `.cpp` ファイルを追加し、`_DEBUG` ガードで囲む。

テスト名は英語で書き、日本語はコメントで補足する（コンソール出力の文字化け対策）。

```cpp
#ifdef _DEBUG
#include <ThirdParty/Catch2/catch.hpp>
#include "WorldPos.hpp"

TEST_CASE("WorldPos::ToScreen - far objects have smaller y")
{
    // 奥にあるものほどy座標が小さい
    REQUIRE(...);
}

#endif
```

## _DEBUG ガード

`#ifdef _DEBUG` で囲むことで、Release ビルドにはテストコードが含まれない。

## ログ出力の禁止

テストから呼ばれるコードに `APP_LOG`（`Console <<`）が含まれていると、Siv3D が
コンソールウィンドウを自動で開き、Catch2 の出力がそのウィンドウに吸われて
`run-tests.sh` 側に届かなくなる。

テストコードおよびテストから呼ばれる実装コードでは `APP_LOG` を使わないこと。
