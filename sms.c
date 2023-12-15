#include "sms.h"

// Twilio 계정 정보
const char* TWILIO_ACCOUNT_SID = "blind";
const char* TWILIO_AUTH_TOKEN = "blind";
const char* TWILIO_PHONE_NUMBER = "unknown";
const char* TO_PHONE_NUMBER = "unknown";

// Twilio SMS API 엔드포인트 URL
const char* TWILIO_SMS_URL = "https://api.twilio.com/2010-04-01/Accounts/AC1bfb25aab627bd95b5cd294e6981821d/Messages.json";

// SMS 메시지 내용
const char* SMS_BODY = "아기가 울어요!";

// 콜백 함수
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    // 사용하지 않는 매개변수에 대한 경고를 없애기 위해 매개변수를 사용하지 않음을 명시
    (void)contents;
    (void)userp;
    return size * nmemb;
}


int sendsms() {
    CURL* curl;
    CURLcode res;
    const char* str1 = ":";
    size_t total_length = strlen(TWILIO_ACCOUNT_SID) + strlen(str1) + strlen(TWILIO_AUTH_TOKEN) + 1;
    char* result = (char*)malloc(total_length);
    strcpy(result, TWILIO_ACCOUNT_SID);
    strcat(result, str1);
    strcat(result, TWILIO_AUTH_TOKEN);

    const char* str2 = "To=";
    const char* str3 = "&From=";
    const char* str4 = "&Body=";

    size_t total_length2 = strlen(str2) + strlen(TO_PHONE_NUMBER) + strlen(str3) + strlen(TWILIO_PHONE_NUMBER) + strlen(str4) + strlen(SMS_BODY) + 1;
    char* result2 = (char*)malloc(total_length2);
    strcpy(result2, str2);
    strcat(result2, TO_PHONE_NUMBER);
    strcat(result2, str3);
    strcat(result2, TWILIO_PHONE_NUMBER);
    strcat(result2, str4);
    strcat(result2, SMS_BODY);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Twilio API에 대한 HTTP POST 요청 설정
        curl_easy_setopt(curl, CURLOPT_URL, TWILIO_SMS_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, result2);
        curl_easy_setopt(curl, CURLOPT_USERPWD, result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        // HTTP POST 요청 수행
        res = curl_easy_perform(curl);

        // 결과 확인
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return -1;
        }

        // curl 리소스 정리
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
