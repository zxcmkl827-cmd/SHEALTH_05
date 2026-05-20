# SHealth BMI QA 최종 보고서

## 1. Executive Summary

본 보고서는 QA 리드 엔지니어 관점에서 Gilded Rose C++ 실습 기반의 SHealth BMI 프로젝트 QA 활동을 종합 검토한 결과이다. 검토 범위는 요구사항 분석, 코드 품질 분석, 테스트 계획, 단위 테스트, Golden Master 회귀 테스트, 결함 관리 문서, 실제 CTest/gcov-lcov 실행 결과를 포함한다.

현재 프로젝트는 기존 placeholder 실패 테스트 상태에서 벗어나 Google Test 20개와 Approval/Golden Master 테스트 1개를 갖춘 자동화 회귀 체계를 확보했다. 최신 로컬 검증 기준 `ctest`는 21/21 Green이며, GCC/gcov 기반 제품 코드 라인 커버리지는 91.5%로 전체 목표 90%를 달성했다.

다만 `SHealth.cpp` 단일 파일 라인 커버리지는 90.4%로 목표 95%에 미달했고, lcov 결과에서 branch coverage는 수집되지 않았다. 테스트 계획서의 P0/P1/P2 시나리오 전체 기준으로는 자동화가 아직 완결되지 않았으며, 파일 open 실패, malformed CSV, 헤더/빈 파일, 반복 호출 상태 초기화 같은 방어 경로 보강이 다음 우선순위이다.

## 2. 테스트 완료율과 커버리지

### 2.1 테스트 실행 결과

검증 명령:

```powershell
ctest --test-dir build --output-on-failure
ctest --test-dir build-coverage --output-on-failure
```

실행 결과:

| 항목 | 결과 | 판단 |
|---|---:|---|
| CTest 전체 테스트 | 21개 |
| Google Test BMI 단위 테스트 | 20개 통과 |
| Approval/Golden Master 테스트 | 1개 통과 |
| 실패 테스트 | 0개 |
| 실행 테스트 통과율 | 100% | 목표 달성 |

현재 자동화 테스트는 BMI 범주별 경계값, 일부 나이 경계, 체중 0 보정, 음수 체중, 키 0/음수, 콘솔 출력 회귀를 검증한다. 이는 핵심 계산 회귀 방지에는 효과적이다.

다만 `docs/test_plan.md`의 명시 시나리오 기준으로 보면 완료율은 다음과 같이 평가된다.

| 기준 | 자동화 완료 | 계획 수 | 완료율 | 판단 |
|---|---:|---:|---:|---|
| P0 | 3 | 5 | 60.0% | 반복 호출/unknown type 테스트 보강 필요 |
| P1 | 8 | 12 | 66.7% | malformed CSV, 파일 없음, 빈 줄 테스트 필요 |
| P2 | 0 | 6 | 0.0% | 정책성/특이 케이스는 미자동화 |
| 전체 계획 시나리오 | 11 | 23 | 47.8% | 실행 Green과 별개로 테스트 설계 대비 미완료 |

### 2.2 gcov/lcov 커버리지

측정 방식:

```powershell
cmake -S . -B build-coverage -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_CXX_COMPILER="C:\mingw64\winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-14.0.0-r7\mingw64\bin\g++.exe" `
  -DCMAKE_CXX_FLAGS="--coverage -O0 -g" `
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"

cmake --build build-coverage
ctest --test-dir build-coverage --output-on-failure
lcov --capture --directory build-coverage --output-file build-coverage\coverage.info
lcov --extract build-coverage\coverage.info "*/src/main/cpp/*" --output-file build-coverage\coverage.product.info
lcov --list build-coverage\coverage.product.info
```

제품 코드 기준 결과:

| 대상 | 목표 | 실제 | 차이 | 판단 |
|---|---:|---:|---:|---|
| 전체 제품 코드 라인 | 90% 이상 | 91.5% (140/153) | +1.5%p | 달성 |
| `SHealth.cpp` 라인 | 95% 이상 | 90.4% (122/135) | -4.6%p | 미달 |
| `StandardBmiClassifier.cpp` 라인 | 100% | 100% (6/6) | 0%p | 달성 |
| 제품 코드 함수 | 별도 목표 없음 | 100% (20/20) | - | 양호 |
| 분기 커버리지 | 85% 이상 | no data found | 측정 불가 | 수집 옵션 개선 필요 |

`SHealth.cpp` 미달 원인은 테스트가 정상 계산과 주요 경계값에 집중되어 있고, 파일 open 실패, 컬럼 부족, 숫자 변환 예외, 알 수 없는 BMI 타입, 빈 파일/헤더만 있는 파일, 반복 호출 같은 방어 분기가 충분히 실행되지 않았기 때문이다.

## 3. 결함 패턴 분석

### 3.1 등록 결함 기준

`docs/defect_list.md` 기준 등록 결함은 3건이다.

| 분류 | 건수 | 주요 내용 |
|---|---:|---|
| Major / TestFailure | 1 | placeholder `FAIL()` 테스트로 CTest가 항상 실패하던 문제. 실제 20개 BMI 테스트 구현으로 해결 |
| Info / DefectCandidate | 1 | 요청된 실패 로그가 현재 저장소에서 재현되지 않은 결함 후보 |
| Info / Tooling | 1 | PowerShell 환경에서 Unix식 `&&` 명령 사용으로 발생한 절차성 문제 |

Severity별로는 Major 1건, Info 2건이다. 제품 코드의 현재 재현 결함은 없지만, 등록 taxonomy에 `DefectCandidate`가 포함되어 있어 `docs/defect_report.md`의 ItemType 정의와 불일치한다. 향후에는 `Functional`, `DataValidation`, `TestFailure`, `Tooling`, `Documentation` 중 하나로 정규화하는 것이 좋다.

### 3.2 결함 후보와 리스크 패턴

요구사항 분석과 코드 품질 분석에서 확인된 결함 후보는 다음 패턴으로 모인다.

| ItemType | 주요 패턴 | 심각도 경향 | 현재 상태 |
|---|---|---|---|
| Functional | BMI `25.0` 미분류, 연령대별 비율 산출 오류 가능성 | Major | BMI `25.0`은 비만으로 수정 및 테스트됨 |
| DataValidation | 컬럼 부족, 비숫자 값, 음수 체중, 키 0/음수, 빈 줄, 전체 체중 0 | Major~Critical | 일부 수정 및 테스트됨. malformed CSV 테스트는 부족 |
| TestFailure | placeholder 실패 테스트, Golden Master mismatch 가능성 | Major | 현재 21/21 Green |
| Tooling | PowerShell 명령 호환성, gcov/lcov 별도 빌드 필요, branch coverage 미수집 | Minor~Info | 절차 문서화 필요 |
| Documentation | 음수 나이, 헤더 없는 CSV, quoting 지원 여부, 키 누락 보정 정책 미정 | Info~Minor | 정책 확정 필요 |

가장 많이 반복되는 결함 패턴은 DataValidation이다. 레거시 C++ 코드에서는 CSV 입력 검증 부족이 계산 오류, 예외 전파, 0 나눗셈, 메모리 접근 위험으로 이어지기 쉽다. 이번 리팩토링으로 고정 배열과 주요 분모 0 위험은 줄었지만, 테스트 자동화는 아직 해당 방어 경로 전체를 고정하지 못했다.

## 4. 9단계 QA 활동 평가

| 단계 | 활동 | 효과성 평가 | 근거 |
|---:|---|---|---|
| 1 | 프로젝트 규칙/작업 기준 정리 | 효과적 | 산출물 명명, C++17, Google Test 기준을 일관되게 유지 |
| 2 | 요구사항 분석 | 매우 효과적 | BMI 경계, 파일 입력, 누락 체중, 0 나눗셈, CSV 파싱 리스크를 조기 식별 |
| 3 | 코드 품질 분석 | 매우 효과적 | SRP 위반, 매직 넘버, 고정 배열, long conditional 등 구조적 결함을 수정 우선순위로 전환 |
| 4 | Modern C++ 리팩토링 | 효과적 | `std::vector`, `std::map`, `std::optional`, `BmiClassifier` 도입으로 변경 용이성과 안전성 개선 |
| 5 | 체중 누락 보정 규칙 정리 | 효과적 | 체중 `0`의 같은 연령대 평균 보정 테스트로 핵심 요구사항 고정 |
| 6 | 테스트 계획 수립 | 매우 효과적 | P0/P1/P2, 경계값, gcov/lcov 목표가 명확해져 현재 미달 항목 판단 가능 |
| 7 | Google Test 구현 | 효과적 | placeholder 실패 테스트를 20개 의미 있는 테스트로 대체 |
| 8 | 결함 분석/디버깅 | 보통 | 현재 실패 재현은 없었으나 결함 재현 절차와 심각도 판단 기준을 정리 |
| 9 | Approval/Golden Master | 매우 효과적 | 콘솔 출력 회귀를 1개 테스트로 고정해 리팩토링 안정성 확보 |

가장 효과적이었던 단계는 요구사항 분석, 코드 품질 분석, 테스트 계획, Golden Master이다. 이 단계들은 결함을 늦게 발견하는 대신 설계/테스트 단계에서 위험을 가시화했다.

개선이 필요한 단계는 테스트 구현과 커버리지 측정 자동화이다. 현재 실행 테스트는 모두 통과하지만, 테스트 계획 대비 자동화 완료율은 낮다. 또한 branch coverage가 수집되지 않았고, `CMakeLists.txt`에 `ENABLE_COVERAGE` 옵션이 없어 커버리지 빌드 절차가 재현 가능한 표준으로 고정되지 않았다.

## 5. Cursor AI 활용 효과

### 5.1 정량 요약

| 항목 | Before | After | 효과 |
|---|---:|---:|---|
| 의미 있는 자동화 테스트 | 0개 또는 placeholder 실패 테스트 | 21개 Green | 테스트 기반 회귀 검증 체계 확보 |
| CTest 통과율 | 실패 상태 | 100% | 릴리스 후보 검증 가능 |
| 제품 코드 라인 커버리지 | 미측정 | 91.5% | 전체 90% 목표 달성 |
| `StandardBmiClassifier.cpp` 라인 커버리지 | 미측정 | 100% | BMI 경계 로직 완전 실행 |
| 등록 결함 해결 | placeholder 실패 1건 | Resolved | 테스트 실패 차단 해소 |
| QA 산출물 | 개별 코드/README 중심 | 요구사항, 품질, 테스트, 결함, Golden Master 문서화 | 추적성 향상 |

시간 단축은 별도 time tracker가 없으므로 추정치로만 판단한다. README의 생성형 AI 활동 계획은 6시간이며, 이 시간 안에서 요구사항 분석, 코드 품질 분석, 리팩토링 방향, 테스트 계획, 테스트 구현, 결함 관리, Golden Master까지 산출했다. 동일 범위를 수작업으로 진행할 경우 보통 1.5~2일 수준의 분석/문서화/테스트 작성이 필요하므로, 실습 범위에서는 약 50~65%의 리드타임 단축 효과가 있었다고 볼 수 있다.

### 5.2 정성 요약

Cursor AI는 결함 조기 발견에 특히 효과적이었다. BMI `25.0` 경계 오류, 분모 0, 컬럼 부족, 비숫자 입력, 키 0, 고정 배열 10,000건 한계 같은 위험을 테스트 작성 전에 요구사항/코드 리뷰 단계에서 드러냈다.

또한 리팩토링 범위를 작은 책임 단위로 나누는 데 도움이 됐다. `calculateBmi`를 오케스트레이터로 축소하고, 파싱, 보정, BMI 계산, 집계를 분리한 구조는 테스트 가능성과 커버리지 측정 가능성을 높였다.

한계도 명확하다. AI가 생성한 테스트는 핵심 경계값에는 강했지만 테스트 계획 전체를 자동으로 완결하지는 못했다. 특히 malformed CSV, 파일 없음, 빈 파일, 상태 초기화, branch coverage 수집 같은 운영 품질 항목은 QA 리드가 목표 대비 갭을 다시 점검해야 했다.

## 6. 다음 레거시 프로젝트 Best Practice 5가지

1. 요구사항을 먼저 테스트 가능한 경계값으로 바꾼다. BMI `18.5`, `23.0`, `25.0`, 나이 `20/29/30/79/80`, 체중 `0`, 키 `0`처럼 계산 규칙과 입력 정책을 표로 고정해야 테스트 누락을 줄일 수 있다.

2. 레거시 코드는 Golden Master를 먼저 만든 뒤 내부를 바꾼다. 콘솔 출력, 파일 출력, API 응답처럼 사용자 관찰 가능한 결과를 승인 파일로 고정하면 리팩토링 중 의도치 않은 회귀를 빨리 잡을 수 있다.

3. 결함은 Severity와 ItemType을 분리해 관리한다. 기능 오류, 데이터 검증, 테스트 실패, 도구 문제, 문서 문제를 같은 `bug`로만 묶으면 우선순위가 흐려진다. 입력 검증 결함은 특히 Major 이상으로 승격될 가능성이 높다.

4. 커버리지 목표는 CMake/CI에 실행 가능한 옵션으로 고정한다. 문서의 목표만으로는 부족하다. `ENABLE_COVERAGE`, 외부 의존성 제외 규칙, branch coverage 옵션, HTML 리포트 산출 경로를 빌드 설정에 포함해야 한다.

5. AI 산출물은 계획 대비 체크리스트로 검증한다. AI는 빠르게 테스트와 문서를 생성하지만, P0/P1/P2 전체 매핑, 미커버 분기, 정책 미정 항목은 사람이 최종 QA 게이트로 확인해야 한다.

## 7. 최종 판단과 후속 권고

현재 SHealth BMI 프로젝트는 핵심 계산 회귀를 자동화했고, 제품 코드 전체 라인 커버리지 목표도 달성했다. 따라서 리팩토링 안정성은 초기 상태 대비 크게 향상되었다.

릴리스 또는 다음 실습 단계로 넘어가기 전에는 다음 항목을 우선 보강해야 한다.

- `SHealth.cpp` 라인 커버리지를 95% 이상으로 올리기 위해 파일 없음, malformed row, 비숫자 입력, 빈 파일, 헤더만 있는 파일, unknown type, 반복 호출 테스트를 추가한다.
- lcov branch coverage 수집 옵션을 활성화해 분기 커버리지 85% 목표를 실제 측정 가능하게 만든다.
- `CMakeLists.txt`에 `ENABLE_COVERAGE` 옵션을 추가해 Windows/MSVC 빌드와 GCC coverage 빌드를 명확히 분리한다.
- `docs/defect_list.md`의 ItemType을 `docs/defect_report.md` taxonomy와 맞춘다.
- 음수 나이, 헤더 없는 CSV, 표준 CSV quoting, 키 누락 보정의 제품 정책을 요구사항 문서에서 확정한다.
