# SHealth 기능 개선 테스트 계획서

## 1. 목적과 범위

본 문서는 `SHealth` BMI 모듈의 기능 개선 항목에 대한 테스트 계획을 정의한다. 대상 기능은 `docs/feature_requirements_analysis.md`의 요구사항을 기준으로 하며, C++17, Google Test, CMake 환경에서 `TEST_F` 기반 단위 테스트로 검증한다.

테스트 대상 공개 동작은 다음과 같다.

- `SHealth::calculateBmi(const std::string& filename)`의 CSV 로딩, 파싱, 누락값 보정, BMI 계산, 상태 초기화
- `SHealth::getBmiRatio(int ageClass, int type)`의 연령대별 BMI 범주 비율 조회
- 추가 예정 API의 키 누락값 보정, 정상 BMI 사용자 목록 조회, 전체 BMI 범주 비율 조회
- `StandardBmiClassifier`의 BMI 범주 분류 경계값

현재 구현은 체중 `0` 보정과 연령대별 비율 계산을 제공한다. 기능 개선 테스트는 기존 동작 보존을 회귀 테스트로 고정한 뒤, 키 `0` 보정, 정상 BMI 사용자 목록, 전체 BMI 분포 API를 추가 검증하는 순서로 작성한다.

## 2. 테스트 환경

| 항목 | 기준 |
|---|---|
| 언어 | C++17 |
| 빌드 | CMake |
| 단위 테스트 | Google Test `TEST_F` |
| 테스트 실행 | `ctest --test-dir build` 또는 `build/SHealthBMITest` |
| 커버리지 | GCC/MinGW 또는 Clang 기반 `gcov`/`lcov` 권장 |
| 테스트 데이터 | 테스트 fixture에서 임시 CSV 파일을 생성 |

## 3. 테스트 설계 원칙

- 테스트는 구현 세부 함수보다 공개 API로 관찰 가능한 결과를 우선 검증한다.
- `TEST_F` fixture에서 임시 CSV 작성, `SHealth` 생성, 비율 검증 헬퍼를 공유한다.
- 부동소수점 비율과 BMI 계산 결과는 `EXPECT_NEAR`를 사용한다.
- 테스트명은 입력 조건과 기대 동작을 드러내는 `Given_When_Then` 또는 `Function_ReturnsExpected_WhenCondition` 형식을 사용한다.
- 기능 추가 전후의 기존 동작 보존을 위해 회귀 테스트를 P0에 포함한다.
- CSV 파싱 오류, 보정 불가, 분모 0 등 실패 가능성이 높은 입력을 정상 경로와 같은 수준으로 테스트한다.

## 4. TEST_F 기반 단위 테스트 범위와 우선순위

권장 fixture는 다음과 같다.

- `SHealthFeatureTest`: CSV 입력 기반으로 `calculateBmi`, 연령대별 비율, 보정, 조회 API를 검증한다.
- `StandardBmiClassifierTest`: BMI 범주 경계값을 독립적으로 검증한다.
- `SHealthStateTest`: 같은 객체의 반복 호출과 파일 오류 후 상태 초기화를 검증한다.

### P0: 핵심 기능과 회귀 방지

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| FT-P0-001 | `CalculateBmi_ReturnsLoadedRecordCount_ForValidCsv` | 정상 CSV를 읽으면 처리된 레코드 수를 반환한다. |
| FT-P0-002 | `GetBmiRatio_ReturnsAgeClassDistribution_ForMixedCategories` | 특정 연령대에 저체중/정상/과체중/비만이 섞인 경우 범주별 비율을 계산한다. |
| FT-P0-003 | `CalculateBmi_ImputesZeroWeight_WithSameAgeClassAverage` | 체중 `0`은 같은 연령대의 유효 체중 평균으로 보정된다. |
| FT-P0-004 | `CalculateBmi_ImputesZeroHeight_WithSameAgeClassAverage` | 키 `0`은 같은 연령대의 유효 키 평균으로 보정된다. |
| FT-P0-005 | `GetNormalBmiUsers_ReturnsOnlyUsersBetweenNormalBoundaries` | 정상 BMI 사용자 목록은 `18.5 < bmi < 23.0`만 포함한다. |
| FT-P0-006 | `GetOverallBmiDistribution_ReturnsRatios_ForAllValidUsers` | 전체 BMI 범주 비율은 전체 유효 BMI 사용자를 분모로 계산한다. |
| FT-P0-007 | `CalculateBmi_ResetsPreviousState_OnRepeatedCalls` | 같은 `SHealth` 객체로 다시 계산해도 이전 통계와 조회 결과가 남지 않는다. |

### P1: 경계값과 입력 안정성

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| FT-P1-001 | `Classify_ReturnsUnderweight_WhenBmiIs18Point5` | BMI `18.5`는 저체중이다. |
| FT-P1-002 | `Classify_ReturnsNormal_WhenBmiIsJustAbove18Point5` | BMI `18.5` 초과는 정상 범위 하한에 포함된다. |
| FT-P1-003 | `Classify_ReturnsOverweight_WhenBmiIs23Point0` | BMI `23.0`은 과체중이다. |
| FT-P1-004 | `Classify_ReturnsObesity_WhenBmiIs25Point0` | BMI `25.0`은 비만이다. |
| FT-P1-005 | `CalculateBmi_GroupsAge20And29IntoTwenties` | 나이 `20`, `29`는 20대에 포함된다. |
| FT-P1-006 | `CalculateBmi_GroupsAge70And79IntoSeventies` | 나이 `70`, `79`는 70대에 포함된다. |
| FT-P1-007 | `CalculateBmi_ExcludesAge19And80FromAgeClassRatios` | 나이 `19`, `80`은 연령대별 비율에서 제외된다. |
| FT-P1-008 | `CalculateBmi_ReturnsZero_WhenFileDoesNotExist` | 파일 open 실패 시 `0`을 반환하고 조회 결과도 비어 있다. |
| FT-P1-009 | `CalculateBmi_SkipsMalformedRows_WithoutThrowing` | 컬럼 부족, 비숫자 필드, 빈 필드는 예외 없이 스킵된다. |
| FT-P1-010 | `CalculateBmi_SkipsRows_WithNegativeWeightOrHeight` | 음수 체중 또는 음수 키는 유효하지 않은 행으로 처리된다. |

### P2: 정책 확인과 특이 케이스

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| FT-P2-001 | `CalculateBmi_ReturnsZeroRatios_ForHeaderOnlyFile` | 헤더만 있는 CSV는 모든 비율이 `0.0`이다. |
| FT-P2-002 | `CalculateBmi_IgnoresBlankLines_AndContinuesReading` | 빈 줄은 무시하고 이후 유효 행을 처리한다. |
| FT-P2-003 | `CalculateBmi_ExcludesUnimputableZeroWeightRows_FromRatios` | 같은 연령대의 모든 체중이 `0`이면 보정 불가 레코드는 비율에서 제외된다. |
| FT-P2-004 | `CalculateBmi_ExcludesUnimputableZeroHeightRows_FromRatios` | 같은 연령대의 모든 키가 `0`이면 보정 불가 레코드는 비율에서 제외된다. |
| FT-P2-005 | `GetNormalBmiUsers_PreservesCsvIdAndInputOrder` | 정상 BMI 사용자 목록은 CSV ID와 입력 순서를 보존한다. |
| FT-P2-006 | `GetOverallBmiDistribution_ReturnsZero_WhenNoValidBmiExists` | 전체 유효 BMI가 없으면 전체 분포 비율은 모두 `0.0`이다. |
| FT-P2-007 | `GetBmiRatio_ReturnsZero_ForUnknownAgeClassOrType` | 지원하지 않는 연령대 또는 타입 코드는 `0.0`을 반환한다. |

## 5. 경계값 케이스 목록

### 5.1 BMI 분류 경계

| BMI 값 | 기대 범주 | 우선순위 |
|---:|---|---|
| `18.49` | 저체중 | P1 |
| `18.50` | 저체중 | P0 |
| `18.51` | 정상 | P0 |
| `22.99` | 정상 | P0 |
| `23.00` | 과체중 | P0 |
| `24.99` | 과체중 | P1 |
| `25.00` | 비만 | P0 |
| `25.01` | 비만 | P1 |

### 5.2 나이와 연령대

| 나이 | 기대 동작 | 우선순위 |
|---:|---|---|
| `-1` | 레코드가 로드되더라도 연령대별 비율에서는 제외 | P2 |
| `0` | 연령대별 비율 제외 | P2 |
| `19` | 연령대별 비율 제외 | P1 |
| `20` | 20대 포함 | P0 |
| `29` | 20대 포함 | P1 |
| `30` | 30대 포함 | P1 |
| `39` | 30대 포함 | P1 |
| `70` | 70대 포함 | P1 |
| `79` | 70대 포함 | P0 |
| `80` | 연령대별 비율 제외 | P1 |

### 5.3 체중

| 체중 값 | 기대 동작 | 우선순위 |
|---:|---|---|
| `-1.0` | 행 스킵 | P1 |
| `0.0` | 같은 연령대 평균 체중으로 보정 | P0 |
| 같은 연령대 모두 `0.0` | 보정 불가, BMI 통계 제외 | P2 |
| `0.1` | 유효 값으로 BMI 계산 | P2 |
| `60.0` | 유효 값으로 BMI 계산 | P0 |
| `300.0` | 유효 값으로 BMI 계산, 비만 분류 가능성 확인 | P2 |
| 빈 문자열 | 행 스킵 | P1 |
| 비숫자 문자열 | 행 스킵 | P1 |

### 5.4 키

| 키 값 | 기대 동작 | 우선순위 |
|---:|---|---|
| `-1.0` | 행 스킵 | P1 |
| `0.0` | 같은 연령대 평균 키로 보정 | P0 |
| 같은 연령대 모두 `0.0` | 보정 불가, BMI 통계 제외 | P2 |
| `1.0` | 유효 값이나 극단적 BMI로 분류 결과 확인 | P2 |
| `100.0` | cm to m 변환 확인 | P1 |
| `170.0` | 일반 유효 값 | P0 |
| `250.0` | 큰 키 값의 BMI 계산 확인 | P2 |
| 빈 문자열 | 행 스킵 | P1 |
| 비숫자 문자열 | 행 스킵 | P1 |

### 5.5 비율 계산

| 상황 | 기대 동작 | 우선순위 |
|---|---|---|
| 분모 0 | `0.0` 반환, divide-by-zero 없음 | P0 |
| 1건 중 1건 | `100.0` | P0 |
| 2건 중 1건 | `50.0` | P0 |
| 3건 중 1건 | `33.333...`, `EXPECT_NEAR` 사용 | P1 |
| 모든 범주가 1건씩 존재 | 각 `25.0` | P0 |
| 잘못된 BMI 타입 코드 | `0.0` | P1 |

## 6. 예외 및 특이 케이스 목록

| 케이스 | 입력 예 | 기대 동작 | 우선순위 |
|---|---|---|---|
| 파일 없음 | 존재하지 않는 경로 | `calculateBmi`는 `0` 반환, 상태 초기화 | P0 |
| 빈 파일 | 내용 없음 | 예외 없이 0건 처리 | P2 |
| 헤더만 존재 | `id,age,weight,height` | 0건 처리, 모든 비율 `0.0` | P2 |
| 컬럼 부족 | `1,20,60` | 행 스킵 | P1 |
| 추가 컬럼 | `1,20,60,170,extra` | 필수 4개 컬럼만 사용하거나 정책 확정 | P2 |
| 비숫자 나이 | `1,abc,60,170` | 행 스킵 | P1 |
| 비숫자 체중 | `1,20,abc,170` | 행 스킵 | P1 |
| 비숫자 키 | `1,20,60,abc` | 행 스킵 | P1 |
| 음수 체중 | `1,20,-1,170` | 행 스킵 | P1 |
| 음수 키 | `1,20,60,-1` | 행 스킵 | P1 |
| 키 0 | `1,20,60,0` | 같은 연령대 평균 키로 보정, 평균이 없으면 통계 제외 | P0 |
| 체중 0 | `1,20,0,170` | 같은 연령대 평균 체중으로 보정, 평균이 없으면 통계 제외 | P0 |
| 정상 BMI 경계 | BMI `18.5`, `23.0` | 정상 사용자 목록에서 제외 | P0 |
| 반복 호출 | 같은 객체로 다른 파일 2회 계산 | 이전 파일의 비율과 사용자 목록 제거 | P0 |

## 7. 커버리지 목표

커버리지 목표는 기능 개선 완료 기준으로 다음과 같이 설정한다.

| 대상 | 목표 |
|---|---:|
| `src/main/cpp/SHealth.cpp` 라인 커버리지 | 90% 이상 |
| `src/main/cpp/SHealth.cpp` 브랜치 커버리지 | 85% 이상 |
| `StandardBmiClassifier.cpp` 라인 커버리지 | 95% 이상 |
| 신규 기능 코드 라인 커버리지 | 90% 이상 |
| 전체 `src/main/cpp` 라인 커버리지 | 90% 이상 |

커버리지 산정에서 `main` 실행 파일 진입점, 외부 라이브러리, 빌드 산출물은 제외한다. 핵심 기준은 `shealth_lib`에 포함되는 비즈니스 로직이다.

## 8. gcov/lcov 측정 전략

GCC 또는 MinGW 환경에서는 별도 커버리지 빌드 디렉터리를 사용한다.

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build build-coverage
ctest --test-dir build-coverage --output-on-failure
lcov --capture --directory build-coverage --output-file build-coverage/coverage.info
lcov --remove build-coverage/coverage.info "*/_deps/*" "*/src/test/*" "*/SHealthBMI.cpp" --output-file build-coverage/coverage.filtered.info
genhtml build-coverage/coverage.filtered.info --output-directory build-coverage/coverage-html
```

Windows에서 `lcov` 사용이 어렵다면 동일 소스 기준으로 WSL 또는 Linux CI에서 커버리지 잡을 별도로 실행한다. 로컬 Windows에서는 테스트 성공 여부를 우선 확인하고, 커버리지 수치는 GCC 기반 CI 결과를 기준으로 관리한다.

## 9. 커버리지 개선 전략

1. 먼저 P0 테스트를 추가해 정상 흐름, 누락값 보정, 정상 BMI 사용자 조회, 전체 분포 계산의 핵심 라인을 덮는다.
2. 커버리지 리포트에서 미실행 분기를 확인한 뒤 P1 입력 오류 테스트로 `std::stoi`, `std::stod`, 파일 open 실패, 음수 값 스킵 경로를 보강한다.
3. 분모 0, 보정 불가, 잘못된 타입 코드 등 방어 분기는 P2 테스트로 별도 검증한다.
4. private 함수 직접 테스트가 필요해질 정도로 커버리지 사각지대가 크면, 구현을 `CsvHealthRecordReader`, `MissingValueImputer`, `BmiStatisticsService` 같은 작은 클래스나 순수 함수로 분리해 독립 테스트 가능성을 높인다.
5. 비율 계산 테스트는 `EXPECT_NEAR(actual, expected, 1e-6)` 기준을 사용해 부동소수점 오차로 인한 flaky test를 방지한다.
6. 신규 API 추가 시 최소 하나의 정상 케이스, 하나의 빈 결과 케이스, 하나의 경계값 케이스를 함께 추가한다.

## 10. 완료 기준

- P0 테스트가 모두 작성되고 통과한다.
- P1 테스트 중 BMI 경계값, 나이 경계값, malformed CSV 테스트가 통과한다.
- 기능 개선 코드의 라인 커버리지가 90% 이상이다.
- `ctest --test-dir build --output-on-failure`가 성공한다.
- 커버리지 리포트에서 미검증 라인이 남아 있다면 위험도와 제외 사유가 PR 또는 테스트 보고서에 기록된다.
