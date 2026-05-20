# SHealth Feature Defect List

현재까지 기능 개선 테스트 실행과 결함 분석 과정에서 발견된 테스트 실패 및 결함 후보를 정리한다.

최신 확인 기준으로 `src/test/cpp/SHealthBMITest.cpp`의 Google Test 40개와 Golden Master 1개는 모두 통과했으며, 재현 가능한 제품 기능 결함은 없다.

## Defects

| ID | Severity | ItemType | Steps | Expected | Actual | Root Cause | Fix Summary |
|---|---|---|---|---|---|---|---|
| FDEF-001 | Info | Tooling | Windows PowerShell 환경에서 `cmake --build build && ctest --test-dir build --output-on-failure` 실행. | 빌드가 성공하면 이어서 CTest가 실행되어 전체 테스트 결과가 출력되어야 한다. | PowerShell에서 `&&` 토큰 파싱 오류가 발생해 빌드와 테스트가 실행되지 않았다. | 검증 명령이 현재 PowerShell 세션에서 지원되지 않는 셸 문법으로 작성되었다. | PowerShell 호환 명령인 `cmake --build build; if ($LASTEXITCODE -eq 0) { ctest --test-dir build --output-on-failure }`로 절차를 수정했고, 전체 41개 테스트 Green을 확인했다. |
| FDEF-002 | Info | TestFailure | `cmake --build build` 후 `ctest --test-dir build --output-on-failure`로 기능 개선 테스트 전체 실행. | 실패 로그가 있다면 `EXPECT_EQ`의 기대값과 실제값 차이가 재현되어야 한다. | 현재 저장소 상태에서는 실패가 재현되지 않았고, 41/41 테스트가 모두 통과했다. | 요청에 실제 CTest 실패 로그가 포함되지 않았으며, 현재 구현은 BMI 경계값, 결측치 보정, 정상 BMI 사용자 조회, 전체 BMI 비율 조회 테스트를 통과한다. | 코드 변경 없음. 실제 실패 로그가 확보되면 해당 로그 기준으로 기대/실제 차이, 파일명:줄번호, 심각도, 최소 수정 diff를 재분석한다. |

## Verification

PowerShell에서 다음 명령으로 확인했다.

```powershell
cmake --build build; if ($LASTEXITCODE -eq 0) { ctest --test-dir build --output-on-failure }
```

결과:

- 전체 테스트: 41개
- 통과: 41개
- 실패: 0개
- 현재 재현 가능한 제품 코드 결함: 없음
