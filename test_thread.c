#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <softTone.h>
#include <stdio.h>
#include <unistd.h>
#include <softPwm.h>
#include <pthread.h>
#include <stdbool.h>
#include "sms.h"

//스텝모터
int pin_arr[4] = {12, 16, 20, 21};

int one_phase[8][4] = { {1,0,0,0},
                        {1,1,0,0},
                        {0,1,0,0},
                        {0,1,1,0},
                        {0,0,1,0},
                        {0,0,1,1},
                        {0,0,0,1},
                        {1,0,0,1}};

void init_Step()
{
    //4개의 핀을 모두 출력 모드로 설정
    for(int i = 0 ; i< 4; i++)
    {
        pinMode(pin_arr[i], OUTPUT);
    }
    printf("모터 완료");
}

void one_two_Phase_Rotate(int steps, int dir){
    if(dir == 0){
        for(int i = steps; i>=0; i--){
            digitalWrite(pin_arr[0], one_phase[i%8][0]);
            digitalWrite(pin_arr[1], one_phase[i%8][1]);
            digitalWrite(pin_arr[2], one_phase[i%8][2]);
            digitalWrite(pin_arr[3], one_phase[i%8][3]);
            delay(3);
        }
    }
    else if(dir == 1){
        for(int i = 0; i<steps; i++){
            digitalWrite(pin_arr[0], one_phase[i%8][0]);
            digitalWrite(pin_arr[1], one_phase[i%8][1]);
            digitalWrite(pin_arr[2], one_phase[i%8][2]);
            digitalWrite(pin_arr[3], one_phase[i%8][3]);
            delay(3);
        }
    }
}

void one_two_Phase_Rotate_Angle(int angle, int dir){
    int steps = (angle * 64) / 5.625;
    one_two_Phase_Rotate(steps, dir);
}

//RGB_LED
#define Red 14
#define Green 15
#define Blue 18
#define VCC 3

void ledInit(void){
    softPwmCreate(Red, 0, 100);
    softPwmCreate(Green, 0, 100);
    softPwmCreate(Blue, 0, 100);

    printf("전구 완료");
}

void SetColer(int r_val, int g_val, int b_val){
    // 3색을 전부 255로 설정시 무드등 꺼짐
    r_val = (r_val / 255.0) * 100;
    g_val = (g_val / 255.0) * 100;
    b_val = (b_val / 255.0) * 100;

    softPwmWrite(Red, r_val);
    softPwmWrite(Green, g_val);
    softPwmWrite(Blue, b_val);
}

//스피커
#define TOTAL_NOTES 50
#define SPEAKER 23
int notes[TOTAL_NOTES] = {
    0, 261, 293, 329, 349, 391, 440, 493, 523, 587, 659
};

int melody[63] = {329, 329, 391, 0, 329, 329, 391, 0, 329, 391,523, 493, 440, 440, 391, 0,
293, 329, 349, 293, 0, 293, 329, 349, 293, 0, 293, 349, 493, 440, 391, 493, 523, 0,
261, 261, 523, 440, 349, 391, 0,
329, 261, 349, 391, 440, 391, 0,
261, 261, 523, 440, 349, 391, 0,
329, 261, 349, 391, 329, 293, 261, 0,
};

int delays[63] = {
500, 500, 1000, 125, 500, 500, 1000, 125, 500, 500, 500, 500, 500, 500, 1000, 125,
500, 500, 500, 1000, 125, 500, 500, 500, 1000, 125, 500, 500, 250, 250, 500, 500, 1000, 125,
250, 500, 500, 500, 500, 1000, 125,
250, 250, 500, 500, 500, 1000, 125,
250, 500, 500, 500, 500, 1000, 152,
250, 250, 250, 250, 500, 500, 1000, 125,
};


void changeTone(int gpio, int note, int del){
    softToneWrite (gpio, note);
    delay(del);
    softToneWrite (gpio, 0);
    delay(100);
}

//센서
#define DEVICE_ADDR 0x48
static const char *I2C_DEV = "/dev/i2c-1";
#define CHANNEL 0

// 공유 변수
int baby_check = 0;  // 아기 울음 여부
int now_count = 0;   // 현재 아기 울음 감지 횟수
int cur = 0;         // 현재 감지 횟수
int sms_sent = 0;    // SMS 전송 여부
int sms_cur = 0;

// 동기화를 위한 뮤텍스와 조건 변수
pthread_mutex_t mutex;
pthread_cond_t cond;

void* babyMonitoring(void* arg) {
    (void)arg;
    int fd;

    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi를 시작할 수 없습니다.\n");
        return NULL;
    }

    if ((fd = wiringPiI2CSetupInterface(I2C_DEV, DEVICE_ADDR)) == -1) {
        printf("I2C 장치를 열 수 없습니다.\n");
        return NULL;
    }

    while (1) {
        wiringPiI2CWrite(fd, 0x40 | CHANNEL);
        int sound_value = wiringPiI2CRead(fd);

        pthread_mutex_lock(&mutex);
        
        if (sound_value > 206) {
            now_count += 1;
        }
        if (now_count > 0) {
            cur += 1;
        }
        if (now_count >= 4 && cur <= 40) {
            baby_check = 1;
            cur = 0;
            now_count = 0;
            pthread_cond_signal(&cond);
        }
        if (now_count < 4 && cur > 40) {
            now_count = 0;
            cur = 0;
        }

        printf("사운드 센서 값: %d 아기 울음 감지 횟수: %d 감지 횟수: %d\n", sound_value, now_count, cur);

        if (baby_check == 1 && sms_sent == 0) {
            //sendsms();
            printf("SMS 전송 완료!\n");
            baby_check = 0;
            sms_sent = 1;
        } 
        else if (baby_check == 1 && sms_sent == 1 && sms_cur > 10000) {
            sms_sent = 0;
            sms_cur = 0;
        }
        if ( sms_cur < 10000 && sms_sent == 1){
            sms_cur += 1;
            printf("sms delay\n");
            delay(500);
        }

        pthread_mutex_unlock(&mutex);

        delay(100);  // 0.1초 대기
    }

    return NULL;
}

//엑추에이터

void *actuator1(void *arg){
    (void)arg;
    printf("speaker 실행");
    softToneCreate (SPEAKER);
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi를 시작할 수 없습니다.\n");
        return NULL;
    }
    
    while(TRUE){
        while (baby_check == 0) {
        }

        for (int count = 0; count < 63; count++){
            changeTone(SPEAKER, melody[count], delays[count]);
        }
    }
}

void *actuator2(void *arg){
    (void)arg;
    printf("LED 실행");

    ledInit();

    while(TRUE){

        SetColer(255,255,255);
        while (baby_check == 0) {
        }

        for(int cycle = 0; cycle < 3; cycle++){
            for (int i = 255; i > 0; i--){
                SetColer(127+i/2,255,i);
                delay(10);
            }
            for (int i = 0; i < 256; i++){
                SetColer(127+i/2,255,i);
                delay(10);
            }
            for (int i = 255; i > 0; i--){
                SetColer(127+i/2,255,i);
                delay(10);
            }
            for (int i = 0; i < 256; i++){
                SetColer(127+i/2,255,i);
                delay(10);
            }
        }
    }
}

void *actuator3(void *arg){
    (void)arg;
    printf("moter 실행");

    init_Step();
    while (TRUE)
    {
        while (baby_check == 0) {
        }
    
        one_two_Phase_Rotate_Angle(180, 1);
    }
}

int main(void) {
    pthread_t thread;
    pthread_t Actuator1, Actuator2, Actuator3;

    // 뮤텍스와 조건 변수 초기화
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("뮤텍스 초기화에 실패했습니다.\n");
        return 1;
    }
    if (pthread_cond_init(&cond, NULL) != 0) {
        printf("조건 변수 초기화에 실패했습니다.\n");
        return 1;
    }


    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi를 시작할 수 없습니다.\n");
        return -1;
    }

    // 아기 모니터링 스레드 생성 및 detached 상태로 만듦
    if (pthread_create(&thread, NULL, babyMonitoring, NULL) != 0) {
        printf("스레드 생성에 실패했습니다.\n");
        return 1;
    }
    if (pthread_detach(thread) != 0) {
        printf("스레드를 detached 상태로 만들지 못했습니다.\n");
        return 1;
    }

    if (pthread_create(&Actuator1, NULL, actuator1, NULL) != 0) {
        printf("스레드 1 생성에 실패했습니다.\n");
        return 1;
    }
    if (pthread_detach(Actuator1) != 0 ) {
        printf("스피커를 detached 상태로 만들지 못했습니다.\n");
        return 1;
    }

    if (pthread_create(&Actuator2, NULL, actuator2, NULL) != 0) {
        printf("스레드 2 생성에 실패했습니다.\n");
        return 1;
    }
    if (pthread_detach(Actuator2) != 0 ) {
        printf("LED를 detached 상태로 만들지 못했습니다.\n");
        return 1;
    }

    if (pthread_create(&Actuator3, NULL, actuator3, NULL) != 0) {
        printf("스레드 3 생성에 실패했습니다.\n");
        return 1;
    }
    if (pthread_detach(Actuator3) != 0 ) {
        printf("모터를 detached 상태로 만들지 못했습니다.\n");
        return 1;
    }

    // 메인 스레드는 다른 작업 수행
    while (1) {
        pthread_mutex_lock(&mutex);
        while (baby_check == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        //printf("main에서 울음감지\n");
        baby_check = 0;
        pthread_mutex_unlock(&mutex);

        delay(1000);  // 1초 대기
    }

    // 뮤텍스와 조건 변수 제거
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}