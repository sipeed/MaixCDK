
#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"


/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_image.hpp"
#include "maix_display.hpp"
#include "maix_lvgl.hpp"
#include "lvgl.h"
#include <unistd.h>
#include "ui.h"
#include "maix_pinmap.hpp"
#include "maix_gpio.hpp"
#include "maix_audio.hpp"
#include <pthread.h>
#include <semaphore.h>
#include "main.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <atomic>
#include "ui_helpers.h"
#include <vector>
#include <string>
#include "maix_err.hpp"
#include <iostream>

using namespace maix;
std::string path_record = "";
int sample_rate_record = 48000, channel_record = 1,record_ms_record = 5000;
audio::Format format_record = audio::Format::FMT_S16_LE;
audio::Recorder r = audio::Recorder(path_record, sample_rate_record, format_record, channel_record);
audio::Player   p = audio::Player(path_record, sample_rate_record, format_record, channel_record);
extern void start_playing(void); 
pthread_t record_thread;
pthread_t play_thread;

static pthread_mutex_t mutex_lvgl;

pthread_mutex_t test_flag;
pthread_mutex_t record_mutex;

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

lv_obj_t* m_list_audio_list;
std::atomic<bool> exit_record(false);
std::atomic<bool> flag_pause_record(false);



std::atomic<bool> is_paused_play(false);
std::atomic<bool> is_play(false);
// bool is_play=false;
// bool is_paused_play=false;
lv_obj_t* btn_list;
static lv_obj_t* currentButton = NULL;
lv_obj_t * label_play;
extern lv_obj_t * ui_Label8;
int list_number_name=0;
int test_flag_audio=0;

//bool
extern bool flag_record;
extern bool flag_play;

extern bool test_my;


audio_node *head_audio=NULL;
audio_node *current_audio=NULL;


audio_node* findNode(audio_node *head, const char *data_item) 
{
    audio_node *currentNode = head;
    while (currentNode != NULL) 
    {
        if (strstr(currentNode->name_audio, data_item)!=NULL) 
        {
  
            return currentNode; 
        }
        currentNode = currentNode->next;
    }
    return NULL; 
}

// void audio_list_delete(audio_node **audio_head, char *name_delete)
// {
// 	audio_node *current = 
// 	strncpy(newnode->name_audio, name_insert,strlen(name_insert)+1);
//     newnode->next=*audio_head;
//     (*audio_head)=newnode;
// }

//new
void msgbox_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_current_target_obj(e);
    lv_obj_t * mbox = lv_obj_get_parent(lv_obj_get_parent(btn));
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    char *label_char=lv_label_get_text(label);
    if(code == LV_EVENT_CLICKED)
    {
        // 根据点击的按钮进行相应操作
        if(strcmp(label_char, "Play") == 0)
        {
            current_audio=findNode(head_audio,lv_list_get_button_text(m_list_audio_list, (lv_obj_t*)e->user_data));
            if (current_audio!=NULL)
            {
                /* code */
                lv_label_set_text(ui_Label8, lv_list_get_button_text(m_list_audio_list, (lv_obj_t*)e->user_data));
                pthread_create(&play_thread, NULL, thread_play_func, NULL);
                pthread_detach(play_thread);
            
                
                _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen3_screen_init);
            }
            
            

            lv_msgbox_close(mbox);// 如果需要，可以在此处关闭消息框
        }
        else if(strcmp(label_char, "Delete") == 0)
        {
            char *path_audio_delete=(char *)malloc(sizeof(char)*100);
            sprintf(path_audio_delete,"/maixapp/share/wav/%s.wav", lv_list_get_button_text(m_list_audio_list, (lv_obj_t*)e->user_data));
            remove(path_audio_delete);
            free(path_audio_delete);
            lv_obj_del((lv_obj_t*)e->user_data);
            lv_msgbox_close(mbox);// 如果需要，可以在此处关闭消息框
        }

    }
}




void event_handler_list(lv_event_t  * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target_obj(e);
    lv_obj_add_state(obj,LV_STATE_FOCUS_KEY);   //添加聚焦状态
    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_t * mbox_main = lv_msgbox_create(ui_Screen1);
        lv_msgbox_add_title(mbox_main, "Title");
        lv_msgbox_add_text(mbox_main, "Do you want to play or delete the audio file?");
        lv_msgbox_add_close_button(mbox_main);
        lv_obj_t * btn_mbox_main_item;
        btn_mbox_main_item = lv_msgbox_add_footer_button(mbox_main, "Play");
        lv_obj_add_event_cb(btn_mbox_main_item, msgbox_event_handler, LV_EVENT_CLICKED, obj);
        btn_mbox_main_item = lv_msgbox_add_footer_button(mbox_main, "Delete");
        lv_obj_add_event_cb(btn_mbox_main_item, msgbox_event_handler, LV_EVENT_CLICKED, obj);
        if (currentButton == obj)
        {
            currentButton = NULL;
        }
        else
        {
            currentButton = obj;
        }
    }
}

void audio_list_insert(audio_node *&audio_head, const char *name_insert)
{
	audio_node *newnode = (audio_node *)malloc(sizeof(audio_node));
	strncpy(newnode->name_audio, name_insert,strlen(name_insert)+1);
    newnode->next=audio_head;
    (audio_head)=newnode;
}
// void audio_list_insert(audio_node *&audio_head, const char *name_insert)
// {
// 	audio_node *newnode = (audio_node *)malloc(sizeof(audio_node));
// 	strncpy(newnode->name_audio, name_insert,strlen(name_insert)+1);
//     if(audio_head==NULL)
//     {
//         audio_head=newnode;
//         return;
//     }
//     audio_node* current = audio_head;
//     while (current->next != nullptr) {
//         current = current->next;
//     }
//     current->next = newnode;
//     newnode->next= NULL;
// }
void load_album(audio_node *&head)
{
    DIR *dp = opendir("/maixapp/share/wav");	//打开home目录
    vector<string> filenames;
    string filename;
    struct dirent *pdir;
    while(pdir = readdir(dp))	
	{
		if(pdir->d_type == DT_REG)				//判断是否为普通文件
		{
			if(strstr(pdir->d_name, ".wav"))	//判断是否为jpg文件
			{
                filename=strtok(pdir->d_name, ".");
                filenames.push_back(filename);
				// char *newname=(char *) malloc(sizeof(char)*100);
                // sprintf(newname,"%s", strtok(pdir->d_name, "."));
				// sprintf(newname,"/root/pcm/%s", pdir->d_name);
				// audio_list_insert(head, newname);//将该文件名称插入链表中
                // btn_list= lv_list_add_btn(m_list_audio_list,LV_SYMBOL_AUDIO,newname);
                // lv_obj_add_event_cb(btn_list, event_handler_list, LV_EVENT_CLICKED, NULL);
                // free(newname);
			}
		}
	}
    closedir(dp);
    std::sort(filenames.begin(), filenames.end(), [](const std::string& a, const std::string& b) {
        // 假设前缀格式为 "YYYYMMDD_HHMMSS"，直接比较字符串即可
        return a < b;
    });
    for (auto& filename : filenames) 
    {
        const char*name=filename.c_str();
        audio_list_insert(head,name);
        btn_list= lv_list_add_btn(m_list_audio_list,LV_SYMBOL_AUDIO,name);
        lv_obj_add_event_cb(btn_list, event_handler_list, LV_EVENT_CLICKED, NULL);
    }


}
void lv_audio_list_init(void)
{
    /*Create a list*/
    m_list_audio_list = lv_list_create(ui_Screen1);
    lv_obj_set_size(m_list_audio_list, 552, lv_pct(80));
    lv_list_add_text(m_list_audio_list,"record");
    currentButton = lv_obj_get_child(m_list_audio_list, 0);
    load_album(head_audio);
}

void *thread_record_func(void *arg)
{
    exit_record=true;
    // pthread_mutex_lock(&mutex_lvgl);
    // lv_imagebutton_set_state(ui_Image3, LV_IMAGEBUTTON_STATE_RELEASED);
    // pthread_mutex_unlock(&mutex_lvgl);
    char path_record_save[100]={0};
    char newname[100]={0};
    // char *path_record_save=(char *)malloc(sizeof(char)*100);
    // char *newname=(char *) malloc(sizeof(char)*100);
    time_t now = std::time(NULL);
    now+=8*3600;
    struct tm *t = localtime(&now);
    // strftime(path_record_save, 100+1, "/maixapp/share/wav/%Y%m%d_%H%M%S.wav", t);
    strftime(path_record_save, 100, "/maixapp/share/wav/%Y%m%d_%H_%M_%S.wav", t);
    // strftime(newname,100+1,"%Y%m%d_%H%M%S", t);
    strftime(newname,100,"%Y%m%d_%H_%M_%S", t);
    audio_list_insert(head_audio, newname);//将该文件名称插入链表中

    pthread_mutex_lock(&mutex_lvgl);
    

    lv_label_set_text(ui_Label12,newname);
    btn_list= lv_list_add_btn(m_list_audio_list,LV_SYMBOL_AUDIO,newname);
    lv_obj_add_event_cb(btn_list, event_handler_list, LV_EVENT_CLICKED, NULL);
    pthread_mutex_unlock(&mutex_lvgl);
    r.record_pcm_to_file_keep_pause(std::string(path_record_save), std::ref(exit_record),std::ref(flag_pause_record));
    // free(newname);
    // free(path_record_save);
    pthread_exit(NULL);  
}
void *thread_play_func(void *arg)
{
    // staticmyled=!staticmyled;
    // is_play=true;
    // is_paused_play=false;
    // pthread_mutex_lock(&mutex_lvgl);
    // lv_imagebutton_set_state(ui_Image4, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED);
    // start_playing();
    // pthread_mutex_unlock(&mutex_lvgl);
    char *path_play_save=(char *)malloc(sizeof(char)*100);
    sprintf(path_play_save,"/maixapp/share/wav/%s.wav", current_audio->name_audio);

    FILE * file_audio_play = fopen(path_play_save, "rb");
    fseek(file_audio_play, 0, SEEK_END);
    long  file_audio_play_size = ftell(file_audio_play);
    int audio_second;
    // if((file_audio_play_size-44)<=96000)
    // int audio_second=(file_audio_play_size-44)/96000;
    if((file_audio_play_size - 44) <= 96000) 
    {
        audio_second = 1;
    } 
    else 
    {
        audio_second = (file_audio_play_size - 44 + 96000 - 1) / 96000; // 向上取整
    }
    fseek(file_audio_play, 44, SEEK_SET);

    
    
    // 计算分钟和秒数
    uint32_t minutes_total = audio_second / 60;
    uint32_t seconds_total = audio_second % 60;

    // 更新标签显示
    char buf_total[16];
    snprintf(buf_total, sizeof(buf_total), "%02d:%02d", minutes_total, seconds_total);

    is_play=true;
    is_paused_play=false;

    pthread_mutex_lock(&mutex_lvgl);
    lv_label_set_text(ui_Label10,buf_total);
    lv_slider_set_value(ui_Slider1, 0, LV_ANIM_OFF);
    lv_slider_set_range(ui_Slider1, 0, audio_second);

    lv_imagebutton_set_state(ui_Image4, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED);
    start_playing();

    pthread_mutex_unlock(&mutex_lvgl);

    

    uint8_t buffer[512];
    int read_len = 0;
    p.audio_prepare();
    while ((read_len = fread(buffer, 1, sizeof(buffer), file_audio_play)) > 0&&is_play.load()) 
    {
        Bytes data(buffer, read_len);
        p.play_data(&data);
        if(is_paused_play.load())
        {
            p.audio_drop();
            while (is_paused_play.load()) 
            {
                if(!is_play.load())
                {
                    break;
                }
            }
            p.audio_prepare();
            
        }  
    }
    p.audio_drop();
    fclose(file_audio_play);
    free(path_play_save);
    pthread_mutex_lock(&mutex_lvgl);
    stop_playing();
    lv_slider_set_value(ui_Slider1, audio_second, LV_ANIM_OFF);
    lv_imagebutton_set_state(ui_Image4, LV_IMAGEBUTTON_STATE_RELEASED);
    pthread_mutex_unlock(&mutex_lvgl);
    is_play=false;
    pthread_exit(NULL); 
     
}
void play_wav_thread()
{
    
    pthread_create(&play_thread, NULL, thread_play_func, NULL);
    pthread_detach(play_thread);
}
int _main(int argc, char* argv[])
{
    std::string  path_pcm_save= "/maixapp/share/wav";
    if(maix::fs::exists(path_pcm_save))
    {

    }
    else
    {
        maix::fs::mkdir(path_pcm_save,false,false);
    }
    
    // peripheral::pinmap::set_pin_function("A14","GPIOA14");
    // peripheral::gpio::GPIO gpio_led= peripheral::gpio::GPIO("A14",peripheral::gpio::Mode::OUT);

    
    // gpio_led.value(1);
    // init display
    display::Display screen = display::Display();

    // touch screen
    touchscreen::TouchScreen touchscreen = touchscreen::TouchScreen();

    // init lvgl
    lvgl_init(&screen, &touchscreen);

    // init lvgl ui
    ui_init();
    lv_audio_list_init();
    
    pthread_mutex_init(&mutex_lvgl, NULL);
    pthread_mutex_init(&test_flag, NULL);
    pthread_cond_init(&condition, NULL);

    lv_slider_set_value(ui_Slider_record, r.volume(), LV_ANIM_OFF);
    lv_slider_set_value(ui_Slider2, 32-p.volume(), LV_ANIM_OFF);
    char buf_set[4];
    lv_snprintf(buf_set, sizeof(buf_set), "%d", lv_slider_get_value(ui_Slider_record));
    lv_label_set_text(ui_Label_record_volume, buf_set);
    lv_snprintf(buf_set, sizeof(buf_set), "%d", lv_slider_get_value(ui_Slider2));
    lv_label_set_text(ui_Label11, buf_set);

    

    

    // main ui loop
    while (!app::need_exit())
    {
        
        pthread_mutex_lock(&mutex_lvgl);
        uint32_t time_till_next = lv_timer_handler();
        pthread_mutex_unlock(&mutex_lvgl);
        time::sleep_ms(time_till_next);
        
    }

    time::sleep_ms(10);

    pthread_mutex_destroy(&mutex_lvgl);
    // pthread_mutex_destroy(&test_flag);
    // pthread_cond_destroy(&condition);

    lvgl_destroy();

    return 0;
}


int main(int argc, char* argv[])
{
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig){ app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

