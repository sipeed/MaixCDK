# WavFileReader
A simple and easy C++ single-header library for loading WAV files. 

## What You Can Do with This
* Loading PCM data as single channel or multi channels 
* Loading data from a random point in file 
* Getting WAV format information of file 

## How To Use
1. Include `wav_file_reader.h`.  
1. Create `sakado::WavFileReader` object and use `Read()` function to read data from WAV files.  

The way of using it is very similar to  `fread()` function in C language.  
```
#include"wav_file_reader.h"

void example() {
	unsigned char buf[44100];
	sakado::WavFileReader wfr("test.wav");
	
	wfr.Read(buf, 44100);

	return;
}
```
Contained class member public functions are as follows.

* `Read()`
* `ReadLR()`
* `Seek()`
* `Tell()`  

See `sample.cpp` for more details.

## Note
* Supported WAV format are 8bit and 16bit.
* `Read()` function accepts several different data types. Convenient.
* Second arg of `Read()` function is not a size of the data but a count of elements of the data.
* The loaded data will be automatically converted and rescaled from signed 16bit to unsigned 8bit by `Read()` function ONLY WHEN the WAV format is 16bit and the prepared buffer is 8bit.
* `Read()` function automatically averages left and right channels if the format is Stereo. You can use `ReadLR()` function if you want to load channels separately.
* `Read()` function returns (the total number of samples successfully read)/(NumChannels). Use it to check EOF.
* `WavFileReader()` constructor throws WFRException object defined in `wav_file_reader.h` when some exception occured.
* `Seek()` function returns the same value as `fseek()` function in `stdio.h` or `cstdio`.
