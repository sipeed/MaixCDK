#ifdef SAKADO_WAV_FILE_READER_H
#else
#define SAKADO_WAV_FILE_READER_H

#ifdef _MSC_VER
#pragma warning(disable:4996)//Without this, Visual Studio throws warnings that suggests to use fopen_s() instead of fopen()
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>

using namespace std;

namespace sakado {
	#define SEEK_SET0	0	/* Seek from beginning of file.  */
	#define SEEK_CUR0	1	/* Seek from current position.  */
	#define SEEK_END0	2	/* Seek from end of file.  */
	class WFRException :public std::exception {};

	class WFRFileOpenException :public  WFRException {
		const char* what(void) const noexcept {
			return "File Open Failed";
		}
	};

	class WFRFileValidityException :public  WFRException {
		const char* what(void) const noexcept {
			return "Invalid File";
		}
	};

	class WFRSupportedFormatException :public  WFRException {
		const char* what(void) const noexcept {
			return "Unsupported format";
		}
	};


	template<typename T> struct ReadFunctionArgumentTypeValidation;
	template<> struct ReadFunctionArgumentTypeValidation<unsigned char> {};
	template<> struct ReadFunctionArgumentTypeValidation<int> {};
	template<> struct ReadFunctionArgumentTypeValidation<short> {};
	template<> struct ReadFunctionArgumentTypeValidation<long> {};
	template<> struct ReadFunctionArgumentTypeValidation<double> {};
	template<> struct ReadFunctionArgumentTypeValidation<float> {};


	class WavFileReader {

	public:
		unsigned long FmtSize;
		unsigned short FmtID;
		unsigned short NumChannels;
		unsigned long SampleRate;
		unsigned long BytesPerSec;
		unsigned short BlockAlign;//(bytes per sample)*(channels)
		unsigned short BitsPerSample;
		unsigned long DataSize;
		unsigned short BytesPerSample;
		unsigned long NumData;


		WavFileReader(string filename) {
			numPrimaryBuf = 65536;//default
			WavFileReaderPrivate(filename.c_str());
		}

		
		WavFileReader(string filename, unsigned int numPrimaryBuf) {
			this->numPrimaryBuf = numPrimaryBuf;
			WavFileReaderPrivate(filename.c_str());
		}

		
		WavFileReader(const WavFileReader&obj) {

			*this = obj;

			fp = fopen(obj.filename, "rb");
			if (fp == NULL) {
				throw WFRFileOpenException();
			}
			fseek(fp, ftell(obj.fp), SEEK_SET0);

			primaryBuf = (unsigned char*)malloc(obj.BytesPerSample * obj.numPrimaryBuf * obj.NumChannels);
			ucharp = (unsigned char*)primaryBuf;
			shortp = (signed short*)primaryBuf;
		}


		~WavFileReader() {
			fclose(fp);
			free(primaryBuf);
		}

		
		void PrintHeader() {

			printf("format size= %d\nformat ID = %d\nchannels = %d\nsampling rate = %d\nbytes per sec = %d\nblock align = %d\nbits per sample = %d\ndata size = %d\n"
				, (int)FmtSize, (int)FmtID, (int)NumChannels, (int)SampleRate, (int)BytesPerSec, (int)BlockAlign, (int)BitsPerSample, (int)DataSize);

			return;
		}

		
		template <class Type> unsigned int Read(Type *buf, unsigned int count) {

			unsigned int readCnt, numSuccess, numDone = 0,leftToRead;

			// sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have a compiler error here, then the type of the first argument of Read function is illegal
			//Available types are [unsigned char*, int*, short*, long*, double*, float*]
			
			leftToRead = count;

			if ((NumChannels == 1 && BitsPerSample == 16 && sizeof(Type) == sizeof(short)) ||
				(NumChannels == 1 && BitsPerSample == 8 && sizeof(Type) == sizeof(unsigned char))) 
			{
				numSuccess = fread(buf, BytesPerSample, leftToRead, fp);
				wavFilePointer += numSuccess;
				return numSuccess;
			}


			if (wavFilePointer + leftToRead > NumData)leftToRead = NumData - wavFilePointer;
			readCnt = numPrimaryBuf * NumChannels;

			while (leftToRead > 0) {

				if (leftToRead < numPrimaryBuf) {
					readCnt = leftToRead * NumChannels;
					leftToRead = 0;
				}
				else {
					leftToRead -= numPrimaryBuf;
				}

				numSuccess = fread(primaryBuf, BytesPerSample, readCnt, fp) / NumChannels;
				if (numSuccess < readCnt) leftToRead = 0;

				ArrangeReadData(buf, numSuccess);
				numDone += numSuccess;
				
			}
			wavFilePointer += numDone;
			return numDone;
		}


		template <class Type> unsigned int ReadLR(Type *bufL, Type *bufR, unsigned int count) {

			unsigned int readCnt, numSuccess, numDone = 0,leftToRead;

			sizeof(ReadFunctionArgumentTypeValidation<Type>);
			//If you have a compiler error here, then the type of the first argument of ReadLR function is illegal
			//Available types are [unsigned char*, int*, short*, long*, double*, float*]

			leftToRead = count;

			if ((NumChannels == 1 && BitsPerSample == 16 && sizeof(Type) == sizeof(short)) ||
				(NumChannels == 1 && BitsPerSample == 8 && sizeof(Type) == sizeof(unsigned char)))
			{
				numSuccess = fread(bufL, BytesPerSample, leftToRead, fp);
				memcpy(bufR, bufL, numSuccess * BytesPerSample);
				wavFilePointer += numSuccess;
				return numSuccess;
			}

			if (wavFilePointer + leftToRead > NumData)leftToRead = NumData - wavFilePointer;
			readCnt = numPrimaryBuf * NumChannels;

			while (leftToRead > 0) {

				if (leftToRead < numPrimaryBuf) {
					readCnt = leftToRead * NumChannels;
					leftToRead = 0;
				}
				else {
					leftToRead -= numPrimaryBuf;
				}

				numSuccess = fread(primaryBuf, BytesPerSample, readCnt, fp) / NumChannels;
				if (numSuccess < readCnt) leftToRead = 0;

				ArrangeReadLRData(bufL, bufR, numSuccess);
				numDone += numSuccess;
				
			}
			wavFilePointer += numDone;
			return numDone;
		}

		
		int Seek(long offset, int origin) {
			if (origin == SEEK_SET0)wavFilePointer = 0;
			if (origin == SEEK_END0)wavFilePointer = NumData - 1;
			if ((long)wavFilePointer < (-1) * offset) {
				fseek(fp, (-1) * wavFilePointer * BlockAlign, origin);
				wavFilePointer = 0;
				return 1;
			}
			if ((unsigned long)(wavFilePointer + offset) > NumData) {
				fseek(fp, (NumData - wavFilePointer)*BlockAlign, origin);
				wavFilePointer = NumData;
				return 1;
			}
			wavFilePointer += offset;
			return fseek(fp, BlockAlign*offset, origin);
		}

		
		unsigned long Tell() {
			return wavFilePointer;
		}

		
	private:
		FILE* fp;
		unsigned int numPrimaryBuf;//count of general purpose buf = total buf size / BlockAlign
		void *primaryBuf;
		unsigned long wavFilePointer;
		unsigned char *ucharp;
		signed short *shortp;
		char filename[256];
		
		void WavFileReaderPrivate(const char*filename) {

			char ch[5];
			unsigned int size,i;//chunk size

			i = 0;
			while (filename[i] != '\0'&&i < 255) {
				this->filename[i] = filename[i];
				i++;
			}
			this->filename[i] = '\0'; 

			fp = fopen(filename, "rb");
			if (fp == NULL) {
				throw WFRFileOpenException();
			}

			fread(ch, 1, 4, fp);
			ch[4] = '\0';
			if (strcmp(ch, "RIFF")) {
				fclose(fp);
				throw WFRFileValidityException();
			}

			fseek(fp, 4, SEEK_CUR0);
			fread(ch, 1, 4, fp);
			ch[4] = '\0';
			if (strcmp(ch, "WAVE")) {
				fclose(fp);
				throw WFRFileValidityException();
			}

			fseek(fp, 4, SEEK_CUR0);
			fread(&FmtSize, 4, 1, fp);
			fread(&FmtID, 2, 1, fp);
			fread(&NumChannels, 2, 1, fp);
			fread(&SampleRate, 4, 1, fp);
			fread(&BytesPerSec, 4, 1, fp);
			fread(&BlockAlign, 2, 1, fp);
			fread(&BitsPerSample, 2, 1, fp);
			fseek(fp, (int)FmtSize - 16, SEEK_CUR0);
			fread(ch, 1, 4, fp);
			ch[4] = '\0';
			while (strcmp(ch, "data")) {
				if (fread(&size, 4, 1, fp) != 1) {
					fclose(fp);
					throw WFRFileValidityException();
				}
				fseek(fp, size, SEEK_CUR0);
				fread(ch, 1, 4, fp);
			}
			fread(&DataSize, 4, 1, fp);
			BytesPerSample = BitsPerSample / 8;
			NumData = DataSize / BlockAlign;

			if (BitsPerSample != 8 && BitsPerSample != 16) {
				fclose(fp);
				throw WFRSupportedFormatException();
			}

			if (NumChannels != 1 && NumChannels != 2) {
				fclose(fp);
				throw WFRFileValidityException();
			}

			wavFilePointer = 0;

			primaryBuf = (unsigned char*)malloc(BytesPerSample * numPrimaryBuf * NumChannels);
			ucharp = (unsigned char*)primaryBuf;
			shortp = (signed short*)primaryBuf;

			return;
		}


		template <class Type> void ArrangeReadData(Type *&buf, unsigned int numSuccess) {
			
			unsigned int i;

			if (NumChannels == 1 && BitsPerSample == 8) {
				for (i = 0; i < numSuccess; i++) {
					*(buf++) = ucharp[i];
				}
			}
			else if (NumChannels == 1 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; i++)
							*(buf++) = (unsigned long)((long)shortp[i] + 0x8000) >> 9;
						break;
					default:
						for (i = 0; i < numSuccess; i++)
							*(buf++) = shortp[i];
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 8) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; i += 2)
							*(buf++) = ((unsigned long)ucharp[i] + (unsigned long)ucharp[i + 1]) >> 1;
						break;
					default:
						for (i = 0; i < numSuccess; i += 2)
							*(buf++) = ((long)ucharp[i] + (long)ucharp[i + 1]) / 2;
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; i += 2)
							*(buf++) = (unsigned long)((long)shortp[i] + (long)shortp[i + 1] + 0x10000) >> 9;
						break;
					default:
						for (i = 0; i < numSuccess; i += 2)
							*(buf++) = ((long)shortp[i] + (long)shortp[i + 1]) / 2;
				}
			}
		}


		template <class Type> void ArrangeReadLRData(Type *&bufL, Type *&bufR, unsigned int numSuccess) {
			
			unsigned int i;

			if (NumChannels == 1 && BitsPerSample == 8) {

				for (i = 0; i < numSuccess; i++) {
					*(bufL++) = ucharp[i];
					*(bufR++) = ucharp[i];
				}
			}
			else if (NumChannels == 1 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; i++) {
							*(bufL++) = (unsigned long)((long)shortp[i] + 0x8000) >> 8;
							*(bufR++) = (unsigned long)((long)shortp[i] + 0x8000) >> 8;
						}
											   break;
					default:
						for (i = 0; i < numSuccess; i++) {
							*(bufL++) = shortp[i];
							*(bufR++) = shortp[i];
						}
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 8) {

				for (i = 0; i < numSuccess; i += 2) {
					*(bufL++) = ucharp[i];
					*(bufR++) = ucharp[i + 1];
				}
			}
			else if (NumChannels == 2 && BitsPerSample == 16) {

				switch (sizeof(Type)) {
					case sizeof(unsigned char) :
						for (i = 0; i < numSuccess; i += 2) {
							*(bufL++) = ((long)shortp[i] + 0x8000) >> 8;
							*(bufR++) = ((long)shortp[i + 1] + 0x8000) >> 8;
						}
											   break;
					default:
						for (i = 0; i < numSuccess; i += 2) {
							*(bufL++) = shortp[i];
							*(bufR++) = shortp[i + 1];
						}
				}
			}
		}

	};

	
}


#endif
