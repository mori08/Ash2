const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

// バッククォートで囲まれた内容が「ルート相対パス（+ 任意の行指定）」なら
// リンク化する。行指定は `:12` または `:12-34` の形式。
// 例: `src/System/Helper.cpp:18-19`
const PATH_SPEC = /^([\w][\w./-]*\.[\w]+)(?::(\d+)(?:-(\d+))?)?$/;

// document のあるディレクトリから上へ辿り、`.git` を持つディレクトリ
// （＝リポジトリのルート）を探す。見つからなければ null。
function findRepoRoot(startFsPath) {
  let dir = path.dirname(startFsPath);
  for (;;) {
    if (fs.existsSync(path.join(dir, '.git'))) {
      return dir;
    }
    const parent = path.dirname(dir);
    if (parent === dir) {
      return null;
    }
    dir = parent;
  }
}

/** @param {vscode.TextDocument} document */
function provideDocumentLinks(document) {
  // plan ファイル（plan-*.md）だけを対象にする
  if (!/^plan-.*\.md$/.test(path.basename(document.uri.fsPath))) {
    return [];
  }
  // ワークスペースの開き方に依存せず、plan ファイルから .git を辿って解決する
  const root =
    findRepoRoot(document.uri.fsPath) ||
    (vscode.workspace.getWorkspaceFolder(document.uri) || {}).uri?.fsPath;
  if (!root) {
    return [];
  }
  const text = document.getText();
  const links = [];

  // 全てのインラインコード span を走査する（matchAll なので lastIndex 状態を持たない）
  for (const m of text.matchAll(/`([^`\n]+)`/g)) {
    const inner = m[1];
    const spec = PATH_SPEC.exec(inner);
    if (!spec) {
      continue;
    }
    const rel = spec[1];
    const abs = path.join(root, rel);
    // 実在するファイルだけをリンク化する（パスでないインラインコードは素通り）
    if (!fs.existsSync(abs)) {
      continue;
    }

    // バッククォートの内側だけを範囲にする
    const startOffset = m.index + 1;
    const endOffset = startOffset + inner.length;
    const range = new vscode.Range(
      document.positionAt(startOffset),
      document.positionAt(endOffset)
    );

    let target = vscode.Uri.file(abs);
    if (spec[2]) {
      // #L12 で該当行へジャンプする
      target = target.with({ fragment: 'L' + spec[2] });
    }
    const link = new vscode.DocumentLink(range, target);
    link.tooltip = rel;
    links.push(link);
  }

  return links;
}

/** @param {vscode.ExtensionContext} context */
function activate(context) {
  context.subscriptions.push(
    vscode.languages.registerDocumentLinkProvider(
      // markdown 全体に登録し、対象の絞り込みは provideDocumentLinks 内で行う
      { language: 'markdown' },
      { provideDocumentLinks }
    )
  );
}

function deactivate() {}

module.exports = { activate, deactivate };
