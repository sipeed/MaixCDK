#include"wav_file_reader.h"
#include<cstdlib>


void example() {

	unsigned char buf[44100];
	
	sakado::WavFileReader wfr("test.wav");

//Load first 44100 samples to buf
//Be aware that second arg is not a size but a count of elements
//Loaded values will be (L+R)/2 if the WAV format is Stereo Channels 
	wfr.Read(buf, 44100);

//Load next 44100 samples to buf
	wfr.Read(buf, 44100);

	return;
}


void advanced_example() {

//The performance will be the best if the second arg of the constructor is set to the same value as which is used when loading  
//The arg is related to the size of the internal buffer
	sakado::WavFileReader wfr("test.wav",1000);

//You can also load left and right data separately
//If the format is Mono, same values will be loaded to bufL and bufR
	unsigned char bufL[1000];
	unsigned char bufR[1000];

	wfr.ReadLR(bufL, bufR, 1000);

//You can use WavFileReader.Seek() function like fseek() function in stdio.h or cstdio
//The first arg is not a bytes-based size but a samples-based size
//If the format is Stereo, the file pointer will jump to (first arg)*2 samples ahead
	wfr.Seek(5000, SEEK_CUR);
	
//Tell() function is almost same as ftell() function in C language.
//Return value is sample-based as Seek() function.
	int v0 = wfr.Tell();
	
//Read() function and ReadLR() function has several overloads
//unsigned char, signed short, int, double, float are available
	int bufInt[1000];
	double bufDouble[1000];
	
	wfr.Read(bufInt, 1000);
	wfr.Read(bufDouble, 1000);

//You can access to some format information 
	int v1 = wfr.NumChannels;
	int v2 = wfr.SampleRate;
	int v3 = wfr.BitsPerSample;
	int v4 = wfr.DataSize;//Total size of data
	int v5 = wfr.NumData;//Total number of samples(if Stereo, L and R samples are counted as 1)

	return;
}


int main(void) {

//This prgram only do loading data from file
//So nothing exciting will happen
	example();
	advanced_example();
	
	return 0;
}


