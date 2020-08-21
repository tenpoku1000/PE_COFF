
# PE_COFF

独自の再配置情報を付加した独自仕様の 64 ビット WebAssembly を生成し、PE/COFF や PE32+(UEFI アプリ)に変換するバックエンド試作版です。
アセンブラ・逆アセンブラ・リンカも自作しています。出力した UEFI アプリは、実機(GIGABYTE GA-Z270X-UD5 + Core i3-7100)で動作しました。  

C 言語で書かれていて、Visual Studio の標準 C ライブラリと Windows API に依存しています。

## ファイルのダウンロード時の注意事項

コマンドプロンプトで以下のコマンドを投入し、ゾーン情報を削除します。

```
C:\>echo.>PE_COFF-master.zip:Zone.Identifier
```

## 開発環境

* Visual Studio Community 2019  
https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
* 64 ビット版 Windows 10

## 出力される UEFI アプリに相当する C 言語のソースコード

以下のソースコードはコンパイルされません。このソースコード相当の内容を C 言語の関数で独自拡張の WebAssembly を生成します。

[efi_main.c](efi_main.c)

## JIT 実行するアプリに相当するソースコード

```
int32_t value1 = (1 + 2) * 3;
int32_t value2 = 2 + (3 * value1);
value1 = value2 + 100;
```

## bin フォルダのコマンドの実行方法

上述したように、ゾーン情報を削除しておくことが前提条件です。

* UEFI アプリを出力する場合：wasm64.cmd を実行すると、以下のファイルが出力されます。
```
PE_COFF_log.log : log ファイル
efi_main.bin : COFF オブジェクトファイル efi_main.obj の .text セクションの Raw Data です。
efi_main.obj : COFF オブジェクトファイル
efi_main_obj.txt : efi_main.obj の機械語を逆アセンブルしたファイルです。
efi_main.txt : efi_main.obj の内容をテキスト化したファイルです。
bootx64.bin : bootx64.efi の .text セクションの Raw Data です。
bootx64_bin.txt : bootx64.bin の機械語を逆アセンブルしたファイルです。
bootx64.efi : UEFI アプリの PE32+ Image ファイルです(efi_main.obj を link したものです)。
bootx64.txt : bootx64.efi の内容をテキスト化したファイルです。
bootx64.wasm : 独自の再配置情報を付加した独自仕様の 64 ビット WebAssembly
bootx64_wasm.txt : bootx64.wasm の Code セクション内の命令列をテキスト化したファイルです。
bootx64_obj.bin : efi_main.bin と同様ですが、JIT のように COFF を出力しない場合でも出力されます
bootx64_obj.txt : bootx64_obj.bin の機械語を逆アセンブルしたファイルです。
```

* 実機での動作確認用：efi_copy_app.cmd を実行すると、bootx64.efi が以下にコピーされます。
```
bin\efi\boot\bootx64.efi : UEFI アプリ(efi_main.obj を link した結果)
```

* JIT 実行の場合：jit.cmd を実行すると、以下のファイルが出力されます。
```
PE_COFF_log.log : log ファイル
bootx64.wasm : WebAssembly(独自の再配置情報なし。32 ビット・アドレッシング)
bootx64_wasm.txt : bootx64.wasm の Code セクション内の命令列をテキスト化したファイルです。
bootx64_obj.bin : JIT コンパイラで生成された機械語です。
bootx64_obj.txt : bootx64_obj.bin の機械語を逆アセンブルしたファイルです。
```

## UEFI アプリでの実機での動作確認

* FAT32 でフォーマットした UEFI メモリに efi\boot\bootx64.efi をコピー
* UEFI セットアップ画面で USB メモリからの起動を選択
* UEFI アプリが起動し、メッセージが表示される
* 任意のキーを押下すると PC がリセットされる。これにより、UEFI アプリが正常に動作したことが確認できる

## ビルド方法

* PE_COFF.sln ファイルをダブルクリックします。
* ゾーン情報を削除していない場合は、Visual Studio のセキュリティ警告を回避してプロジェクトを開きます。  
![warning](images/MSVC.PNG)

* F7 キーを押下します。

ビルド後に tools フォルダに存在するファイルが、bin フォルダにコピーされます。

## ライセンス

[MIT license](LICENSE)

## 作者

市川 真一 <tenpoku1000@outlook.com>

