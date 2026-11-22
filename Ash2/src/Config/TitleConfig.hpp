#pragma once
#include <Siv3D.hpp>

/// @brief タイトル画面の配置・色の設定値
struct TitleConfig {
  /// ロゴの中心の y 座標（論理座標、px）
  double logoY;
  /// 1つ目のボタンの中心の y 座標（論理座標、px）
  double menuY;
  /// ボタンの中心同士の縦の間隔（px）
  double menuSpacing;
  /// ボタンの当たり範囲の幅（px）
  double buttonWidth;
  /// ボタンの当たり範囲の高さ（px）
  double buttonHeight;
  /// 背景（画面全体）の色
  ColorF backgroundColor;
  /// 水面線の色
  ColorF surfaceLineColor;
  /// ロゴに乗算する色
  ColorF logoColor;
  /// 選択されていないボタンの文字色
  ColorF buttonNormalColor;
  /// 選択中のボタンの文字色
  ColorF buttonFocusedColor;
  /// 水面線の中心の y 座標（論理座標、px）
  double surfaceY;
  /// 水面線の太さ（px）
  double surfaceLineWidth;

  /// @brief TOML からタイトル画面の設定を生成する
  /// @throw Error 必須キーの欠落・型違いがあるとき
  [[nodiscard]] static TitleConfig FromToml(const TOMLValue& toml);
};
