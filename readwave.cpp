#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include "readwave.h"
#include "portaudio.h" 

//判断文件是否是一个windows wave file，并用后者接收前者
bool WaveRewind(FILE *wav_file, WavFileHead *wavFileHead)
{
	char riff[8], wavefmt[8];  //riff[4]??
	short i;
	rewind(wav_file);
	fread(wavFileHead, sizeof(struct WavFileHead), 1, wav_file);

	for (i = 0; i<8; i++)
	{
		riff[i] = wavFileHead->RIFF[i];
		wavefmt[i] = wavFileHead->WAVEfmt_[i];
	}
	riff[4] = '\0';
	wavefmt[7] = '\0';
	if (strcmp(riff, "RIFF") == 0 && strcmp(wavefmt, "WAVEfmt") == 0)
		return	true;  // It is WAV file.
	else
	{
		rewind(wav_file);
		return(false);
	}
}


//判断无误后将数据存入buffer——waveData
short *ReadWave(char *wavFile, int *numSamples, int *sampleRate)
{
	FILE	*wavFp;
	WavFileHead		wavHead;
	short	*waveData;
	long	numRead;

	wavFp = fopen(wavFile, "rb");
	if (!wavFp)
	{
		printf("\nERROR:can't open %s!\n", wavFile);
		exit(0);   //exit（0）：正常运行程序并退出程序；
		//exit（1）：非正常运行导致退出程序；
	}

	if (WaveRewind(wavFp, &wavHead) == false)
	{
		printf("\nERROR:%s is not a Windows wave file!\n", wavFile);
		exit(0);
	}

	waveData = new short[wavHead.RawDataFileLength / sizeof(short)];   //file length
	numRead = fread(waveData, sizeof(short), wavHead.RawDataFileLength / 2, wavFp);  //除以2？？
	assert(numRead * sizeof(short) == (unsigned long)wavHead.RawDataFileLength); //如果它的条件返回错误，则终止程序执行
	fclose(wavFp);

	*numSamples = wavHead.RawDataFileLength / sizeof(short);
	*sampleRate = wavHead.SampleRate;
	return	waveData;
}

//构建好header后复制给buffer
void FillWaveHeader(void *buffer, int raw_wave_len, int sampleRate)
{
	WavFileHead  wavHead;

	strcpy(wavHead.RIFF, "RIFF");
	strcpy(wavHead.WAVEfmt_, "WAVEfmt ");
	wavHead.FileLength = raw_wave_len + 36;
	wavHead.noUse = 16;
	wavHead.FormatCategory = 1;
	wavHead.NChannels = 1;
	wavHead.SampleRate = sampleRate;
	wavHead.SampleBytes = sampleRate * 2;  //=sampleRate * BytesPerSample
	wavHead.BytesPerSample = 2;
	wavHead.NBitsPersample = 16;
	strcpy(wavHead.data, "data");
	wavHead.RawDataFileLength = raw_wave_len;

	memcpy(buffer, &wavHead, sizeof(WavFileHead));
	//从源src所指的内存地址的起始位置开始拷贝n个字节到目标dest所指的内存地址的起始位置中
}

void WriteWave(char *wavFile, short *waveData, int numSamples, int sampleRate)
{
	FILE	*wavFp;
	WavFileHead		wavHead;
	long	numWrite;

	wavFp = fopen(wavFile, "wb");
	if (!wavFp)
	{
		printf("\nERROR:can't open %s!\n", wavFile);
		exit(0);
	}

	FillWaveHeader(&wavHead, numSamples*sizeof(short), sampleRate);
	fwrite(&wavHead, sizeof(WavFileHead), 1, wavFp);  //将wavHead里的内容写入wavFile
	numWrite = fwrite(waveData, sizeof(short), numSamples, wavFp);   //write两遍？？
	assert(numWrite == numSamples);
	fclose(wavFp);
}

void GetWavHeader(char *wavFile, short *Bits, int *Rate,
	short *Format, int *Length, short *Channels)
{
	FILE	*wavFp;
	WavFileHead		wavHead;
	char    *waveData;
	long	numRead, File_length;

	wavFp = fopen(wavFile, "rb");
	if (!wavFp)
	{
		printf("\nERROR:can't open %s!\n", wavFile);
		exit(0);
	}
	fseek(wavFp, 0, SEEK_END);   //wavFp指向文件尾
	File_length = ftell(wavFp);  //ftell得到文件位置指针当前位置相对于文件首的偏移字节数
	//由于wavFp指向文件尾，所以可以得到文件长度
	if (WaveRewind(wavFp, &wavHead) == false)
	{
		printf("\nERROR:%s is not a Windows wave file!\n", wavFile);
		exit(0);
	}

	waveData = new char[(File_length - sizeof(struct WavFileHead)) / sizeof(char)];//减去文件头的长度
	numRead = fread(waveData, sizeof(char), File_length - sizeof(struct WavFileHead), wavFp);
	//如果调用成功返回实际读取到的项个数，waveData接收wavFp？？使用fread？？
	fclose(wavFp);

	*Bits = wavHead.NBitsPersample;
	*Format = wavHead.FormatCategory;
	*Rate = wavHead.SampleRate;
	*Length = numRead;
	*Channels = wavHead.NChannels;

	delete[]	waveData;
}


short *ReadWavFile(char *wavFile, int *numSamples, int *sampleRate)
{
	FILE	*wavFp;
	WavFileHead		wavHead;
	short	*waveData;
	long	numRead, File_length;

	wavFp = fopen(wavFile, "rb");
	if (!wavFp)
	{
		printf("\nERROR:can't open %s!\n", wavFile);
		exit(0);
	}
	fseek(wavFp, 0, SEEK_END);
	File_length = ftell(wavFp);


	if (WaveRewind(wavFp, &wavHead) == false)
	{
		printf("\nERROR:%s is not a Windows wave file!\n", wavFile);
		exit(0);
	}

	waveData = new short[(File_length - sizeof(struct WavFileHead)) / sizeof(short)];
	numRead = fread(waveData, sizeof(short), (File_length - sizeof(struct WavFileHead)) / sizeof(short), wavFp);
	fclose(wavFp);

	*numSamples = numRead;
	*sampleRate = wavHead.SampleRate;
	return	waveData;
}

void ReadWav(char *wavFile, short *waveData, int *numSamples, int *sampleRate)
{
	FILE	*wavFp;
	WavFileHead		wavHead;
	long	numRead;

	wavFp = fopen(wavFile, "rb");
	if (!wavFp)
	{
		printf("\nERROR:can't open %s!\n", wavFile);
		exit(0);
	}

	if (WaveRewind(wavFp, &wavHead) == false)
	{
		printf("\nERROR:%s is not a Windows PCM file!\n", wavFile);
		exit(0);
	}

	numRead = fread(waveData, sizeof(short), wavHead.RawDataFileLength / 2, wavFp);
	assert(numRead*sizeof(short) == (unsigned long)wavHead.RawDataFileLength);
	fclose(wavFp);

	*numSamples = wavHead.RawDataFileLength / sizeof(short);
	*sampleRate = wavHead.SampleRate;
}

/*int main()
{
	FILE 

}*/
