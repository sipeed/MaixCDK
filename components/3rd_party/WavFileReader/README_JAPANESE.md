みなさんWAVファイルを読み込みたくなる時、ありますよね。<br>
そんな時にわざわざ自分でファイルを解析するプログラムを書くのは面倒だと思いませんか？単に基本的なフォーマット情報や波形を読み込みたいだけなら、C++から使えるこの`WavFileReader`がお手軽です！シングルヘッダライブラリとなっていますのでヘッダファイルをインクルードするだけで使うことができます。えっ？用意するバッファのデータ型やチャンネル数を元ファイルのWAVフォーマットに合わせるのが面倒くさい？大丈夫。適当にメモリを用意しておけば、読み込まれるデータはとりあえず音声データとしての形が破綻しないよう自動的に変換してくれます。**そう、この`WavFileReader`ならね**（詳細は後述太字部）。本ReadMeではこれの使い方をざっくりと説明いたします。ただし対応している形式は非圧縮リニアPCMの8bitと16bitのみであり、24bitには対応していないことにご注意ください。~~また、可読性はユーザビリティとかパフォーマンスとかの犠牲になったのでヘッダファイルの中身は覗かないでください。~~


# 1. ヘッダをインクルードする
https://github.com/KanijiroSakado/WavFileReader
githubからWavFileReaderのリポジトリをcloneもしくはダウンロードし、その中にある`wav_file_reader.h`をこのライブラリを使用したいファイルの先頭でインクルードしてください。

```C++
#include"wav_file_reader.h"
```
　　
# 2. `WavFileReader`のインスタンスを作る

```C++
	sakado::WavFileReader wfr("test.wav");
```
作ってください。名前空間`sakado`をお忘れなく。読み込むファイルもここで指定してください。
　　
# 3. `Read()`関数でPCMデータを読み込む
`template <class Type> unsigned int Read(Type *buf, unsigned int count);`  

読み込むPCMデータを格納する配列を用意してください。その配列の先頭ポインタを第一引数に渡し、第二引数には読み込みたいサンプルの個数を渡します。

これで第一引数の配列には第二引数で指定された個数ぶんのサンプルが格納されます。C言語の`fread()`関数のような感じです。次のサンプルが読み込みたければ`fread()`と同様にもう一度`Read()`関数を呼べば次のデータが格納されます。

```C++
	unsigned char buf[44100];
	wfr.Read(buf, 44100);//最初の44100サンプルを読み込み
	wfr.Read(buf, 44100);//次の44100サンプルを読み込み
```

ここで注意して頂きたいのは**WAVファイルのフォーマットがステレオの場合は、左右のデータが自動的に平均された値が配列に格納される**ということです。左右のデータを別々に取得したい場合は[後述](#readlr関数)する`ReadLR()`関数を使用してください。

`Read()`関数はテンプレート関数になっているため、第一引数には配列ポインタとして`unsigned char*`型以外にも`int*`型、`short*`型、`long*`型、`double*`型、`float*`型が受け取れます。それ以外の型を渡すとエラーになります。

基本的に読み込んだデータは数値としては加工されずにそのまま配列に格納されます。しかし**読み込み先が`16bit`で受け取り先が`8bit`の場合のみ例外で、そのままでは溢れてしまうので値は自動的に`8bit`に変換されて格納されます**。上記の例の場合でも`unsigned char*`型で受け取っているので、読み込みファイルのフォーマットが`16bit`の場合にはこれが発動します。
　　
# 4. コード全体
以上が最も基本的な使い方です。コードの全体図を以下に示します。

```C++
#include"wav_file_reader.h"

int main(void) {

	sakado::WavFileReader wfr("test.wav");
	unsigned char buf[44100];
	
	wfr.Read(buf, 44100);//最初の44100サンプルを読み込み
	wfr.Read(buf, 44100);//次の44100サンプルを読み込み

	return 0;
}
```
簡単ですね。
　　
# 5. おまけ
細かい機能の説明をします。
<br/>

### __WAVフォーマット情報__

インスタンス変数により取得可能です。

```C++
	int v1 = wfr.NumChannels;
	int v2 = wfr.SampleRate;
	int v3 = wfr.BitsPerSample;
	int v4 = wfr.DataSize;//データの総サイズ
	int v5 = wfr.NumData;//データの総サンプル数（ステレオの場合は左右のセットで１つと数えます）
```
<br/>

### __`WavFileReader()`コンストラクタ__  
   
`WavFileReader(const char* filename);`
`WavFileReader(const char* filename, unsigned int numPrimaryBuf);`  

速度面のパフォーマンスを最大限に発揮したいという方は、以下のように第二引数に自分が後にRead()関数の第二引数として使用する値と同じ値またはそれより大きな値を指定してください。これはインスタンス内部で確保されるメモリの大きさに関係します。なので無駄に大きすぎるのも良くないでしょう。

```C++
sakado::WavFileReader wfr("test.wav",44100);
```
また、このコンストラクタはファイルが存在しないなどの例外の場合に`wav_file_reader.h`内で定義されている`WFRException`オブジェクトをスローします。  
<br/>  
  
### __`Read()`関数__
`template <class Type> unsigned int Read(Type *buf, unsigned int count);`  

[上](#3-read関数でpcmデータを読み込む)でだいたい説明しました。戻り値は`fread()`関数と同じく読み込みに成功したサンプル数です。通常は第二引数と同じ値が返ります。  
<br />  
  
### __`ReadLR()`関数__
`template <class Type> unsigned int ReadLR(Type *bufL, Type *bufR, unsigned int count);`  

`Read()`関数とだいたい同じですが、違う点は配列のポインタを2つ受け取るところです。`bufL`に左のPCMデータ、`bufR`に右のPCMデータがそれぞれ`count`個ずつ格納されます。**読み込み先がモノラルデータの場合は2つの配列には同じ値が格納されます**。この関数も`Read()`関数と同様に第一引数に`unsigned char*`以外の型を受け取れます。扱える型の種類も先程[上](#3-read関数でpcmデータを読み込む)で挙げた`Read()`関数と同様です。

戻り値は読み込みに成功したサンプル数です。左右のセットで1つのサンプル数とカウントします。通常は第三引数と同じ値です。

```C++
	unsigned char bufL[1000];
	unsigned char bufR[1000];

	wfr.ReadLR(bufL, bufR, 1000);
```
<br/>

### __`Seek()`関数__
`int Seek(long offset, int origin);`  

C言語の`fseek()`関数とだいたい同じですが、`offset`に指定する値は移動するデータサイズではなくサンプル数です。読み込み先がステレオの場合は`offset`*2サンプル先に進むことになります。

`wav_file_reader`ヘッダ内で`cstdio`をインクルードしているので`origin`には`SEEK_CUR`、`SEEK_SET`、`SEEK_END`が使えます。

戻り値は成功で0、失敗で0以外です。

```C++
	wfr.Seek(5000, SEEK_CUR);
```
<br/>

### __`Tell()`関数__
`unsigned long Tell(void);`  

C言語の`ftell()`関数のようなものです。戻り値は読み込んでいるファイルの現在の読み込み位置です。戻り値の単位は例によってデータサイズではなくサンプル数であることに注意してください。読み込み先がステレオの場合は左右のセットで1サンプルと数えます。

```C++
	unsigned int v0 = wfr.Tell();
```
