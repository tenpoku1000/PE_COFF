
# PE_COFF

MSVC の出力した 1 個の COFF オブジェクトファイルを入力して UEFI アプリを出力する簡易 Linker です。  
出力した UEFI アプリは、実機(GIGABYTE GA-Z270X-UD5 + Core i3-7100)で動作しました。  
使用した MSVC は以下の通りです。

```
Microsoft(R) C/C++ Optimizing Compiler Version 19.23.28106.4 for x64
```

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

## bin フォルダのコマンドの実行方法

Visual Studio 2019 Developer Command Prompt で作業します。  
上述したように、ゾーン情報を削除しておくことが前提条件です。

* PE_COFF.exe を実行すると、以下のファイルが出力されます。
```
efi_main.bin : COFF オブジェクトファイル efi_main.obj の .text セクションの Raw Data です。
efi_main.txt : efi_main.obj の内容をテキスト化したファイルです。
bootx64.efi : UEFI アプリの PE32+ Image ファイルです(efi_main.obj を link したものです)。
bootx64.bin : bootx64.efi の .text セクションの Raw Data です。
bootx64.txt : bootx64.efi の内容をテキスト化したファイルです。
```

* efi_main_dump.cmd を実行すると、以下のファイルが出力されます。
```
efi_main_obj.txt : (efi_main.obj を dumpbin した結果)
```

* bootx64_dump.cmd を実行すると、以下のファイルが出力されます。
```
bootx64_efi.txt : (bootx64.efi を dumpbin した結果)
```

* efi_copy_app.cmd を実行すると、bootx64.efi が以下にコピーされます(実機での動作確認用)。
```
bin\efi\boot\bootx64.efi : UEFI アプリ(efi_main.obj を link した結果)
```

## ビルド方法

* PE_COFF.sln ファイルをダブルクリックします。
* ゾーン情報を削除していない場合は、Visual Studio のセキュリティ警告を回避してプロジェクトを開きます。  
![warning](https://raw.githubusercontent.com/tenpoku1000/PE_COFF/master/images/MSVC.PNG)
* F7 キーを押下します。

ビルド後に tools フォルダに存在するファイルが、bin フォルダにコピーされます。

## ライセンス

[MIT license](https://raw.githubusercontent.com/tenpoku1000/PE_COFF/master/LICENSE)

## 作者

市川 真一 <tenpoku1000@outlook.com>

