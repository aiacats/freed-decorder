# FreeD Decoder

FreeD カメラトラッキングプロトコルを受信・デコードし、OSC (Open Sound Control) で転送する Windows GUI アプリケーション。

## 機能

- **FreeD 受信**: 指定した IP:Port で FreeD (0xD1) パケットを UDP 受信
- **リアルタイムデコード**: Position (X/Y/Z)、Rotation (Pan/Tilt/Roll)、Zoom、Focus を表示
- **OSC 送信**: デコードしたデータを指定先へ OSC で転送
  - **Individual モード**: 各値を個別の OSC メッセージとして送信
  - **Bundled モード**: 全値を 1 メッセージにまとめて送信
- **GUI**: Dear ImGui (DirectX11) によるリアルタイムモニタリング

## スクリーンショット

```
+-- FreeD Input -----+  +-- Live Data Display ----------+
| Bind IP: [0.0.0.0] |  | Camera ID: 1                  |
| Port:    [40000   ] |  | Position       Rotation       |
| [Start/Stop]       |  | X: 1234.56    Pan:   45.12   |
| Status / Stats     |  | Y: -567.89    Tilt: -12.45   |
+--------------------+  | Z:  890.12    Roll:   0.78   |
+-- OSC Output ------+  | Zoom: 2048    Focus: 1024    |
| Dest IP: [127.0.0.1]| +-------------------------------+
| Port:    [9000     ]|
| Prefix:  [/freed   ]|
| Mode: Individual    |
| [Enable/Disable]   |
+--------------------+
```

## 必要環境

- Windows 10/11
- Visual Studio 2022 (C++ デスクトップ開発ワークロード)
- CMake 3.24 以上
- Git

## ビルド

### 簡単な方法

```
build.bat
```

ダブルクリックまたはコマンドプロンプトで実行すると、vcpkg のセットアップからビルドまで自動で行います。

### 手動ビルド

```bash
# vcpkg をクローン (初回のみ)
git clone --depth 1 https://github.com/microsoft/vcpkg.git vcpkg
vcpkg\bootstrap-vcpkg.bat -disableMetrics

# ビルド
set VCPKG_ROOT=%cd%\vcpkg
cmake --preset release
cmake --build build/release --config Release --target FreeDDecoder
```

生成される exe: `build\release\Release\FreeDDecoder.exe`

## 使い方

1. `FreeDDecoder.exe` を起動
2. **FreeD Input** パネルで受信する IP とポートを設定し「Start Listening」をクリック
3. FreeD 対応デバイスからデータが送信されると **Live Data** パネルにリアルタイム表示
4. **OSC Output** パネルで送信先 IP・ポート・プレフィックスを設定し「Enable Sending」をクリック

## OSC アドレス

プレフィックスはデフォルト `/freed` で、GUI から変更可能です。

### Individual モード

| アドレス | 型 | 内容 |
|---------|-----|------|
| `/freed/pos/x` | float | Position X |
| `/freed/pos/y` | float | Position Y |
| `/freed/pos/z` | float | Position Z |
| `/freed/rot/pan` | float | Yaw (deg) |
| `/freed/rot/tilt` | float | Pitch (deg) |
| `/freed/rot/roll` | float | Roll (deg) |
| `/freed/zoom` | float | Zoom (raw) |
| `/freed/focus` | float | Focus (raw) |

### Bundled モード

| アドレス | 型 | 内容 |
|---------|-----|------|
| `/freed/transform` | float x8 | pos_x, pos_y, pos_z, yaw, pitch, roll, zoom, focus |

## FreeD プロトコル仕様

29 バイトの UDP パケット:

| バイト | フィールド | エンコーディング |
|--------|-----------|-----------------|
| 0 | メッセージタイプ | 0xD1 固定 |
| 1 | カメラ ID | uint8 |
| 2-4 | Pitch | 24bit BE signed / 32768 → deg |
| 5-7 | Yaw | 同上 |
| 8-10 | Roll | 同上 |
| 11-13 | Pos Z | 24bit BE signed / 64 |
| 14-16 | Pos Y | 同上 |
| 17-19 | Pos X | 同上 |
| 20-22 | Zoom | 24bit BE signed |
| 23-25 | Focus | 24bit BE signed |
| 26-27 | Reserved | - |
| 28 | Checksum | (0x40 - sum[0..27]) & 0xFF |

## 依存ライブラリ

| ライブラリ | 用途 | 管理方法 |
|-----------|------|---------|
| [Dear ImGui](https://github.com/ocornut/imgui) | GUI | vcpkg |
| [oscpp](https://github.com/kaoskorobase/oscpp) | OSC パケット構築 | CMake FetchContent |
| DirectX 11 | レンダリング | Windows SDK |
| Winsock2 | UDP 通信 | Windows SDK |

## ライセンス

MIT
