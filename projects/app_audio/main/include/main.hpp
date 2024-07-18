#pragma once
#include <stdio.h>
typedef struct audio_node
{
	char name_audio[100];
	struct audio_node *next=NULL; //下一张
	
}audio_node;
void *thread_play_func(void *arg);
extern void stop_playing(void);
extern void start_playing(void);
void *thread_pause_func(void *arg);
void play_wav_thread();
void *thread_play_func(void *arg);