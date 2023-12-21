# 아이 울음 감지 및 대처 시스템 - 최종 보고서

## 목차

- [아이 울음 감지 및 대처 시스템 - 최종 보고서](#아이-울음-감지-및-대처-시스템---최종-보고서)
  - [목차](#목차)
  - [아이디어 소개](#아이디어-소개)
  - [전체 시스템 구조](#전체-시스템-구조)
    - [1. 사운드 감지 모듈](#1-사운드-감지-모듈)
    - [2. 액추에이터 제어 모듈](#2-액추에이터-제어-모듈)
    - [3. 알림 전송 모듈](#3-알림-전송-모듈)
  - [구현 내용](#구현-내용)
    - [4.1 프로그램 흐름](#41-프로그램-흐름)
    - [4.2 제한 조건 구현 내용](#42-제한-조건-구현-내용)
  - [개발과정에서의 문제점 및 해결방안](#개발과정에서의-문제점-및-해결방안)
    - [1. 문제점: FFT 분석의 어려움](#1-문제점-fft-분석의-어려움)
    - [2. 해결방안: 음압 데시벨을 통한 울음소리 감지 로직](#2-해결방안-음압-데시벨을-통한-울음소리-감지-로직)
    - [3. 추가 고려사항: 아두이노 우노 보드를 활용한 아날로그 값 분석](#3-추가-고려사항-아두이노-우노-보드를-활용한-아날로그-값-분석)
  - [가산점 요소 구현](#가산점-요소-구현)
  - [기능별 데모 영상](#기능별-데모-영상)

---

## 아이디어 소개

우리 팀은 아이 울음을 감지하고 부모에게 신속하게 알림을 전송하여 아이를 진정시키고자 하는 아기케어 시스템을 개발하였습니다. 이 시스템은 사운드 센서를 통해 아이 울음소리를 감지하고, 부저를 통한 멜로디 출력, LED를 통한 조명 조절, 스텝 모터를 통한 모빌 동작 등 다양한 액추에이터를 활용하여 아이를 안정화시킵니다. 동시에, 아이 울음 감지 시 SMS를 통해 부모에게 알림을 전송하여 신속한 대처가 가능하도록 합니다.

## 전체 시스템 구조

### 1. 사운드 감지 모듈

- 아이 울음을 사운드 센서를 통해 감지합니다.
- 아날로그 값을 라즈베리파이에서 디지털로 읽어옵니다.

### 2. 액추에이터 제어 모듈

- 아이 울음 감지 시, 부저를 통한 멜로디 출력, LED를 켜고 스텝 모터를 동작시켜 모빌을 회전시킵니다.

### 3. 알림 전송 모듈

- 아이 울음 감지 시, Twilio API를 이용하여 SMS를 부모의 핸드폰으로 전송합니다.

## 구현 내용

### 4.1 프로그램 흐름

1. **음압 센서 체크**
   - 음압 센서를 통해 지속적으로 음압을 체크합니다.
   - 높은 음압이 일정 시간 내에 5회 이상 감지될 경우 아기 울음 소리로 감지되도록 프로그램이 구현되어 있습니다.
   - 5회 이상의 조건은 일시적인 주변 소음에 프로그램이 민감하게 오동작하는 것을 방지하기 위함입니다.

2. **아기 울음 소리 감지**
   - 음압 센서를 통해 아기 울음 소리가 감지되면 Twilio API를 활용하여 SMS를 부모의 핸드폰으로 발송합니다.
   - 동시에 시스템의 액추레이터들이 작동됩니다.

3. **액추레이터 동작**
   - SMS 발송은 한 번의 울음 소리를 감지하면 한 번 발송되고, 발송 후 일정 시간 동안 Delay를 가지게 됩니다.
   - 아기 울음 소리가 감지될 때까지 액추레이터들이 동작하며, 감지 모드로 들어가고 처음의 프로그램 흐름으로 돌아가게 됩니다.

4. **아기 울음 소리 미감지**
   - 아기 울음 소리가 더 이상 감지되지 않으면 액추레이터들의 동작이 천천히 정지됩니다.
   - 다시 음압 센서는 [음압 체크](#41-사운드-감지-모듈)로 진입하며, 프로그램 흐름이 처음으로 돌아가게 됩니다.

이러한 흐름을 통해 시스템은 아기 울음 소리를 효과적으로 감지하고, 부모에게 신속하게 알림을 전송하며, 동시에 다양한 액추레이터들이 동작하여 아이를 안정화시키는 역할을 수행합니다.
### 4.2 제한 조건 구현 내용

저희 조는 Pthread와 mutex를 활용하여 다음과 같은 제한 조건을 구현하였습니다:

1. **멀티 스레드 구현**
   - 아기 울음소리 감지 함수를 자식 스레드에서 멀티 스레드로 구현하였습니다.
   - 이를 통해 지속적인 감지 작업이 메인 스레드를 차단하지 않고 수행될 수 있습니다.

2. **뮤텍스 활용**
   - 뮤텍스를 이용하여 전역 변수의 값을 상호 배제성을 보장하였습니다.
   - 이로써 여러 스레드에서 동시에 해당 변수에 접근하는 것을 방지하고, 안전한 수정 및 공유 자원의 접근이 가능합니다.

3. **동시 실행 제어**
   - 아기 울음소리가 감지될 경우, 해당 값이 변하고 뮤텍스를 이용하여 동시에 여러 액추레이터 실행 함수들이 동작하지 않도록 제한하였습니다.
   - 각 액추레이터 실행 함수들은 동작을 마치면 자식 스레드로 돌아가 아기 울음소리 감지 함수를 다시 멀티 스레딩으로 수행합니다.

이를 통해 시스템은 안전하게 여러 스레드 간의 작업을 조율하고, 공유 자원을 안전하게 활용할 수 있습니다.

## 개발과정에서의 문제점 및 해결방안

### 1. 문제점: FFT 분석의 어려움

맨 처음에는 사운드 센서의 아날로그 값을 FFT를 통해 분석하여 특정 주파수 대역의 소리를 감지하려고 시도했습니다. 그러나 라즈베리파이 개발환경에서 정확한 FFT 분석이 어려웠습니다. 특히, 실험 환경에서 정확한 피크값을 얻기 어려웠고, 특정 주파수에서의 노이즈만 도출되는 문제가 발생했습니다.

### 2. 해결방안: 음압 데시벨을 통한 울음소리 감지 로직

- 원인 분석: 사운드 센서의 아날로그 값의 변동폭이 작아 라즈베리파이에서 FFT 분석에 부적합한 것으로 확인되었습니다. 성인 남성이 큰 소리로 말해도 아날로그 값의 변동이 제한적이었습니다.

- 대안 채택: 이에 대한 대안으로 사운드 센서의 아날로그 값을 직접 FFT 분석하는 것이 아닌, 음압 데시벨을 통한 울음소리 감지 로직으로 변경하였습니다. 이는 라즈베리파이의 개발환경에서 민감한 감지가 가능하도록 하였습니다.

### 3. 추가 고려사항: 아두이노 우노 보드를 활용한 아날로그 값 분석

- 부족한 시간으로 인한 한계: 프로젝트 준비 시간이 부족하여 아두이노 우노 보드를 사용하여 사운드 센서의 아날로그 신호를 분석하고, 이를 라즈베리파이와 연동하는 시도를 하지 못했습니다.

- 향후 발전 가능성: 아두이노 우노 보드를 활용하여 사운드 센서의 아날로그 값을 정확하게 분석하는 방안을 고려할 수 있습니다. 이는 프로젝트를 추가적으로 발전시킬 때 고려해볼 수 있는 옵션입니다.

## 가산점 요소 구현

- **SMS 알림 전송 기능**: Twilio API를 통해 라즈베리파이에서 SMS를 전송하는 기능을 구현하였습니다.

## 기능별 데모 영상

[데모 영상 보기](영상 링크)

---
