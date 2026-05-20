# SHealth Defect List

현재까지 확인된 테스트 실패와 결함 후보를 QA 추적용으로 정리한다. 각 항목은 요청된 형식인 `[ID] [Severity] [ItemType] [Steps] [Expected] [Actual] [Root Cause] [Fix Summary]`를 따른다.

## Defects

| ID | Severity | ItemType | Steps | Expected | Actual | Root Cause | Fix Summary |
|---|---|---|---|---|---|---|---|
| DEF-001 | Major | TestFailure | `cmake --build build` 실행 후 `ctest` 실행. 기존 `src/test/cpp/SHealthBMITest.cpp`에 placeholder 테스트만 있는 상태에서 테스트 수행. | BMI 계산 테스트가 요구사항 기준으로 의미 있는 검증을 수행하고 통과해야 한다. | `TEST(SHealthBMITest, FailedTest) { FAIL(); }` 때문에 CTest가 항상 실패한다. | 실제 기능 검증 테스트가 구현되지 않았고, 의도적으로 실패하는 placeholder 테스트가 남아 있었다. | Resolved. `SHealthBMITest` fixture와 BMI 타입별 20개 `TEST_F`를 추가했고, `ctest --test-dir build --output-on-failure` 기준 20/20 Green을 확인했다. |
| DEF-002 | Info | DefectCandidate | BMI 결함 분석 요청 후 현재 코드에서 `cmake --build build`, `ctest --test-dir build --output-on-failure` 실행. | 실패 로그가 있다면 `EXPECT_EQ` 기대값/실제값 차이가 재현되어야 한다. | 현재 저장소 상태에서는 20개 테스트가 모두 통과하고 `EXPECT_EQ` 실패가 재현되지 않는다. | 요청에 `ctest` 실패 로그가 포함되지 않았고, 현재 구현은 BMI 계산, 분류 경계, 비율 조회 테스트를 통과한다. | No code change. 실패 로그 확보 시 해당 로그 기준으로 기대/실제 차이, 결함 위치, 심각도를 재분석한다. |
| DEF-003 | Info | Tooling | Windows PowerShell 환경에서 `cmake --build build && ctest` 형식으로 Green 확인 명령 실행. | 빌드가 성공하면 이어서 CTest가 실행되어야 한다. | 현재 PowerShell 세션에서 `&&` 토큰 파싱 오류가 발생했다. | 검증 명령이 Unix 계열 셸 또는 `&&` 지원 셸 문법으로 작성되어 현재 실행 환경과 맞지 않았다. | Resolved by procedure. PowerShell에서는 `cmake --build build` 후 `ctest --test-dir build --output-on-failure`를 별도 실행하거나 `$LASTEXITCODE` 조건문을 사용한다. |

## Current Verification

최신 확인 결과:

```powershell
cmake --build build
ctest --test-dir build --output-on-failure
```

결과:

- 전체 20개 테스트 통과
- 실패 테스트 0개
- 현재 재현 가능한 제품 코드 결함 없음
