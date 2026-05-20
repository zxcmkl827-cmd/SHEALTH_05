# SHealth BMI 테스트 계획서

## 1. 목적과 범위

본 문서는 SHealth BMI 계산 모듈의 단위 테스트, 경계값 테스트, 예외/특이 케이스 테스트, 커버리지 측정 전략을 정의한다.

테스트 대상은 C++17, Google Test, CMake 기반의 다음 컴포넌트이다.

- `SHealth::calculateBmi(const std::string& filename)`
- `SHealth::getBmiRatio(int ageClass, int type)`
- `StandardBmiClassifier::classify(double bmi)`
- CSV 입력 파싱, 체중 누락값 보정, BMI 계산, 연령대별 BMI 범주 비율 산출 흐름

테스트는 구현 세부 함수가 아닌 공개 API에서 관찰 가능한 동작을 우선 검증한다. 단, BMI 분류처럼 독립성이 높은 로직은 `StandardBmiClassifier` 단위 테스트로 직접 검증한다.

## 2. 테스트 환경

| 항목 | 기준 |
|---|---|
| 언어 | C++17 |
| 빌드 | CMake 3.10+ |
| 테스트 프레임워크 | Google Test |
| 실행 방식 | `ctest` 또는 `SHealthBMITest` 직접 실행 |
| 커버리지 도구 | 가능하면 GCC/Clang + gcov/lcov, Windows MSVC 환경에서는 별도 Linux/GCC 커버리지 빌드 권장 |
| 테스트 데이터 | 테스트 fixture가 임시 CSV 파일을 생성하고 테스트 종료 시 정리 |

현재 `CMakeLists.txt`는 `shealth_lib`, `SHealthBMI`, `SHealthBMITest` 타깃을 분리하고 Google Test `v1.14.0`을 `FetchContent`로 내려받는다. `src/test/cpp/SHealthBMITest.cpp`에는 placeholder 실패 테스트만 있으므로, 본 계획의 P0 테스트부터 `TEST_F` 기반 fixture로 교체한다.

## 3. 테스트 설계 원칙

- `TEST_F` 기반 fixture를 기본으로 사용해 임시 파일 생성, CSV 작성, `SHealth` 인스턴스 초기화, 결과 검증 헬퍼를 공유한다.
- `SHealth` 통합성 단위 테스트와 `StandardBmiClassifier` 분류 테스트를 fixture별로 분리한다.
- Given-When-Then 구조를 유지한다.
- 정상 흐름, 경계값, 예외/특이 입력, 회귀 테스트 순서로 추가한다.
- public API 결과인 반환 건수와 `getBmiRatio(ageClass, type)` 값을 중심으로 검증한다.
- 부동소수점 비교는 `EXPECT_DOUBLE_EQ`보다 `EXPECT_NEAR`를 우선 사용한다.
- 현재 구현에서 유효하지 않은 라인은 스킵되므로, 요구사항과 다른 동작은 회귀/결함 후보로 명시하고 기대 동작을 테스트명에 드러낸다.

## 4. TEST_F 기반 단위 테스트 범위와 우선순위

권장 fixture는 다음 두 가지이다.

- `SHealthTest`: 임시 CSV 파일 작성, `calculateBmi`, `getBmiRatio`, 상태 초기화, 파싱/보정 흐름 검증
- `StandardBmiClassifierTest`: BMI 경계값과 범주 분류 검증

### P0: 핵심 정상 흐름

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| UT-P0-001 | `CalculateBmi_ReturnsLoadedRecordCount_ForValidCsv` | 정상 CSV 입력 시 유효 레코드 수를 반환한다. |
| UT-P0-002 | `CalculateBmi_ComputesRatios_ByAgeClassAndBmiType` | 20대, 30대 등 연령대별 저체중/정상/과체중/비만 비율을 계산한다. |
| UT-P0-003 | `CalculateBmi_ImputesZeroWeight_WithSameAgeClassAverage` | 체중 `0`을 같은 연령대의 0이 아닌 체중 평균으로 보정한다. |
| UT-P0-004 | `GetBmiRatio_ReturnsZero_ForUnknownAgeClassOrType` | 존재하지 않는 연령대 또는 BMI 타입 조회 시 `0.0`을 반환한다. |
| UT-P0-005 | `CalculateBmi_ResetsPreviousState_OnRepeatedCalls` | 같은 객체로 재계산해도 이전 비율이 남지 않는다. |

### P1: BMI 분류 경계와 연령대 경계

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| UT-P1-001 | `Classify_ReturnsUnderweight_WhenBmiIs18Point5` | `StandardBmiClassifierTest`에서 BMI `18.5`는 저체중임을 검증한다. |
| UT-P1-002 | `Classify_ReturnsNormal_WhenBmiIsGreaterThan18Point5AndLessThan23` | `StandardBmiClassifierTest`에서 BMI `18.5` 초과, `23` 미만은 정상임을 검증한다. |
| UT-P1-003 | `Classify_ReturnsOverweight_WhenBmiIs23` | `StandardBmiClassifierTest`에서 BMI `23.0`은 과체중임을 검증한다. |
| UT-P1-004 | `Classify_ReturnsObesity_WhenBmiIs25` | `StandardBmiClassifierTest`에서 BMI `25.0`은 비만임을 검증한다. |
| UT-P1-005 | `CalculateBmi_GroupsAges_ByTenYearBuckets` | 나이 `20, 29, 30, 39, 70, 79`가 올바른 연령대로 집계된다. |
| UT-P1-006 | `CalculateBmi_ExcludesAgesOutsideSupportedBuckets` | 나이 `19`, `80`은 20~70대 비율 집계에서 제외된다. |

### P1: 입력 검증과 파싱 안정성

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| UT-P1-007 | `CalculateBmi_ReturnsZero_WhenInputFileDoesNotExist` | 입력 파일이 없으면 `0`을 반환하고 상태를 초기화한다. |
| UT-P1-008 | `CalculateBmi_SkipsMalformedRows_WithMissingColumns` | 컬럼이 부족한 행은 안전하게 스킵된다. |
| UT-P1-009 | `CalculateBmi_SkipsMalformedRows_WithNonNumericAgeWeightOrHeight` | 나이/체중/키가 비숫자이면 해당 행을 스킵한다. |
| UT-P1-010 | `CalculateBmi_SkipsRows_WithNegativeWeight` | 음수 체중 행은 스킵된다. |
| UT-P1-011 | `CalculateBmi_SkipsRows_WithZeroOrNegativeHeight` | 키가 `0` 또는 음수인 행은 스킵된다. |
| UT-P1-012 | `CalculateBmi_IgnoresBlankLines_AndContinuesReading` | 빈 줄은 무시하고 이후 행을 계속 처리한다. |

### P2: 특이 케이스와 정책 확인

| ID | 테스트명 예시 | 검증 내용 |
|---|---|---|
| UT-P2-001 | `CalculateBmi_ReturnsZero_ForHeaderOnlyFile` | 헤더만 있는 파일은 0건 처리되고 모든 비율이 `0.0`이다. |
| UT-P2-002 | `CalculateBmi_ReturnsZero_ForEmptyFile` | 빈 파일은 0건 처리되고 예외 없이 종료된다. |
| UT-P2-003 | `CalculateBmi_AllZeroWeightsInAgeClass_DoNotContributeToRatios` | 같은 연령대의 모든 체중이 `0`이면 보정 불가로 BMI `0`이 되어 비율에 포함되지 않는다. |
| UT-P2-004 | `CalculateBmi_AllowsExtraColumns_AndIgnoresThem` | 5번째 이후 컬럼은 무시하는 현재 정책을 검증한다. |
| UT-P2-005 | `CalculateBmi_HandlesWhitespaceAroundNumericFields` | 숫자 주변 공백이 있는 입력의 현재 허용 동작을 검증한다. |
| UT-P2-006 | `CalculateBmi_TreatsTrailingEmptyFields_AsMalformedWhenRequiredFieldMissing` | 필수 숫자 필드가 비어 있으면 해당 행을 스킵한다. |

## 5. 경계값 케이스 목록

### 나이

| 값 | 기대 동작 | 우선순위 |
|---:|---|---|
| `-1` | 유효 레코드로 로드될 수 있으나 연령대 비율 집계에서는 제외된다. 요구사항상 입력 오류 처리 여부를 확정해야 한다. | P1 |
| `0` | 비율 집계 제외 | P2 |
| `19` | 비율 집계 제외 | P1 |
| `20` | 20대 집계 포함 | P0 |
| `29` | 20대 집계 포함 | P1 |
| `30` | 30대 집계 포함 | P1 |
| `39` | 30대 집계 포함 | P1 |
| `70` | 70대 집계 포함 | P1 |
| `79` | 70대 집계 포함 | P1 |
| `80` | 비율 집계 제외 | P1 |

### 체중

| 값 | 기대 동작 | 우선순위 |
|---:|---|---|
| `-1.0` | 행 스킵 | P1 |
| `0.0` | 같은 연령대의 0이 아닌 체중 평균으로 보정 | P0 |
| 같은 연령대 모두 `0.0` | 보정 불가, BMI `0`, 비율 산출 대상 제외 | P2 |
| 매우 작은 양수 `0.1` | 유효 값으로 BMI 계산 | P2 |
| 일반 값 `60.0` | 유효 값으로 BMI 계산 | P0 |
| 매우 큰 값 `300.0` | 유효 값으로 BMI 계산하되 비만 비율 반영 확인 | P2 |
| 빈 문자열 | 행 스킵 | P1 |
| 비숫자 `abc` | 행 스킵 | P1 |

### 키

| 값 | 기대 동작 | 우선순위 |
|---:|---|---|
| `-170.0` | 행 스킵 | P1 |
| `0.0` | 행 스킵 | P1 |
| 매우 작은 양수 `1.0` | 유효 값으로 BMI 계산하되 비현실 입력 정책 확인 필요 | P2 |
| 일반 값 `170.0` | 유효 값으로 BMI 계산 | P0 |
| 매우 큰 값 `250.0` | 유효 값으로 BMI 계산 | P2 |
| 빈 문자열 | 행 스킵 | P1 |
| 비숫자 `abc` | 행 스킵 | P1 |

### BMI 분류

| BMI | 기대 범주 | 코드 |
|---:|---|---:|
| `18.5` | 저체중 | `100` |
| `18.5001` | 정상체중 | `200` |
| `22.9999` | 정상체중 | `200` |
| `23.0` | 과체중 | `300` |
| `24.9999` | 과체중 | `300` |
| `25.0` | 비만 | `400` |

## 6. 예외/특이 케이스 목록

| 케이스 | 기대 동작 | 검증 포인트 | 우선순위 |
|---|---|---|---|
| 입력 파일 없음 | `calculateBmi`가 `0`을 반환 | 이전 계산 결과가 노출되지 않아야 함 | P1 |
| 디렉터리 경로 입력 | open 실패로 `0` 반환 | 플랫폼별 동작 차이 확인 | P2 |
| 빈 파일 | 0건 처리 | 모든 비율 `0.0`, 예외 없음 | P2 |
| 헤더만 있는 파일 | 0건 처리 | 모든 비율 `0.0`, 예외 없음 | P2 |
| 헤더 없음 | 첫 번째 데이터 행이 헤더로 소비됨 | 현재 동작을 결함 후보로 기록 | P2 |
| 컬럼 4개 미만 | 해당 행 스킵 | 정상 행은 계속 처리 | P1 |
| 추가 컬럼 존재 | 추가 컬럼 무시 | 현재 정책 고정 여부 확인 | P2 |
| 중간 빈 줄 | 빈 줄 스킵 후 계속 처리 | 데이터 절단 없음 | P1 |
| 나이 비숫자 | 해당 행 스킵 | 예외가 외부로 전파되지 않음 | P1 |
| 체중 비숫자 또는 빈 값 | 해당 행 스킵 | 예외가 외부로 전파되지 않음 | P1 |
| 키 비숫자 또는 빈 값 | 해당 행 스킵 | 예외가 외부로 전파되지 않음 | P1 |
| 음수 나이 | 통계 제외 또는 입력 오류 정책 필요 | 현재는 비율 집계 제외 | P1 |
| 음수 체중 | 해당 행 스킵 | 반환 건수와 비율 모두 제외 반영 | P1 |
| 키 0 또는 음수 | 해당 행 스킵 | 0 나눗셈 방지 | P1 |
| 같은 연령대 체중이 모두 0 | 보정하지 않고 비율 제외 | 분모 0 없이 `0.0` 반환 | P2 |
| 알 수 없는 BMI 타입 코드 | `0.0` 반환 | 예: `0`, `500`, `-1` | P1 |
| 알 수 없는 연령대 | `0.0` 반환 | 예: `10`, `80`, `999` | P1 |
| 반복 호출 | 매 호출 시작 시 상태 초기화 | 성공 후 실패, 성공 후 성공 조합 검증 | P0 |

## 7. Fixture 설계안

`SHealthBMITest`는 다음 공통 기능을 가진 fixture로 구성한다.

- 테스트별 고유 임시 파일 경로 생성
- CSV 헤더와 데이터 행 작성 헬퍼 제공
- `SHealth` 인스턴스 생성
- 테스트 종료 시 임시 파일 삭제
- 비율 비교용 허용 오차 상수 제공

예상 구조는 다음과 같다.

```cpp
class SHealthTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    std::filesystem::path writeCsv(std::initializer_list<std::string> rows);
    SHealth shealth;
    static constexpr double RatioTolerance = 1e-9;
};
```

`std::filesystem`은 C++17 표준이므로 임시 경로와 파일 정리에 사용할 수 있다. Windows/MSVC 환경에서 파일 잠금 문제가 발생하지 않도록 테스트 내 파일 스트림은 helper 내부에서 즉시 close되게 작성한다.

BMI 분류 전용 fixture는 상태가 작으므로 다음 정도로 충분하다.

```cpp
class StandardBmiClassifierTest : public ::testing::Test {
protected:
    StandardBmiClassifier classifier;
};
```

## 8. 커버리지 목표

| 대상 | 목표 |
|---|---:|
| 전체 라인 커버리지 | 90% 이상 |
| `SHealth.cpp` 라인 커버리지 | 95% 이상 |
| `StandardBmiClassifier.cpp` 라인 커버리지 | 100% |
| 분기 커버리지 | 85% 이상 |
| 핵심 예외/특이 케이스 | P0/P1 100% 자동화 |

커버리지 수치는 품질의 보조 지표로 사용한다. 단순 실행 커버리지보다 요구사항과 결함 위험을 직접 검증하는 assertion을 우선한다.

## 9. gcov/lcov 측정 전략

MSVC 빌드 산출물은 gcov/lcov와 직접 호환되지 않으므로, 가능하면 GCC 또는 Clang 기반 별도 커버리지 빌드를 사용한다.

프로젝트에 커버리지 옵션을 추가할 경우에는 일반 Debug/Release 빌드와 분리되도록 `ENABLE_COVERAGE` 같은 CMake 옵션을 둔다.

```cmake
option(ENABLE_COVERAGE "Enable gcov/lcov coverage flags" OFF)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(shealth_lib PRIVATE --coverage -O0 -g)
    target_link_options(shealth_lib PRIVATE --coverage)
    target_compile_options(SHealthBMITest PRIVATE --coverage -O0 -g)
    target_link_options(SHealthBMITest PRIVATE --coverage)
endif()
```

### 권장 명령

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_COVERAGE=ON \
  -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"

cmake --build build-coverage
ctest --test-dir build-coverage --output-on-failure

lcov --capture --directory build-coverage --output-file build-coverage/coverage.info
lcov --remove build-coverage/coverage.info \
  "*/_deps/*" "*/src/test/*" "/usr/*" \
  --output-file build-coverage/coverage.filtered.info
genhtml build-coverage/coverage.filtered.info --output-directory build-coverage/coverage-report
```

### 개선 루프

1. P0 테스트를 먼저 작성해 정상 흐름과 상태 초기화를 고정한다.
2. `StandardBmiClassifier`의 BMI 경계값 테스트를 추가해 분류 분기를 100% 커버한다.
3. 파일 open 실패, 잘못된 행, 빈 파일, 헤더만 있는 파일로 파싱과 방어 로직 분기를 채운다.
4. 커버리지 리포트에서 미커버 라인이 남으면 해당 라인이 요구사항상 의미 있는 동작인지 판단한다.
5. 의미 있는 미커버 라인은 테스트를 추가하고, 의미 없는 방어 코드는 제거 또는 구조 개선 후보로 분류한다.
6. P0/P1 테스트가 모두 Green이고 `SHealth.cpp` 95% 이상이면 리팩토링 또는 기능 개선을 진행한다.

## 10. 테스트 완료 기준

- P0/P1 테스트가 모두 자동화되어 Green 상태이다.
- `ctest --output-on-failure`가 통과한다.
- `SHealth.cpp` 라인 커버리지 95% 이상, 전체 라인 커버리지 90% 이상을 달성한다.
- 커버리지 미달성 항목은 남은 리스크와 함께 명시한다.
- 실패 테스트 또는 현재 요구사항과 불일치하는 동작은 결함 후보로 별도 기록한다.

## 11. 주요 리스크와 후속 개선 후보

- 음수 나이는 현재 파싱 단계에서 거부되지 않고 연령대 비율에서만 제외된다. 입력 오류로 볼지, 통계 제외로 볼지 정책 확정이 필요하다.
- 헤더가 없는 파일은 첫 데이터 행이 헤더로 소비된다. 헤더 검증 요구사항이 필요하다.
- 단순 쉼표 split만 지원하므로 표준 CSV quoting, escape, 쉼표 포함 값은 지원하지 않는다.
- 체중 `0` 보정은 같은 연령대 평균만 사용한다. 같은 연령대에 유효 체중이 없으면 보정하지 않는다.
- 키 누락값 보정은 현재 범위 밖이다. README의 개선 항목에 맞춰 별도 기능 요구사항과 테스트가 필요하다.
