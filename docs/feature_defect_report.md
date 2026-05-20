# SHealth 기능 개선 결함 관리 보고서

## 1. 목적과 범위

본 문서는 `SHealth` BMI 기능 개선 작업의 결함 분류, 보고 템플릿, 품질 메트릭 수집 계획을 정의한다.
범위는 `docs/feature_requirements_analysis.md`, `docs/feature_test_plan.md`, `docs/feature_defect_list.md`를 기준으로 하며, C++17, CMake, Google Test, `gcov`/`lcov` 기반 검증 흐름을 대상으로 한다.

현재 기준으로 Google Test 40개와 Golden Master 1개는 모두 통과했으며, 재현 가능한 제품 기능 결함은 없다. 기존 결함 후보는 PowerShell 명령 호환성 등 검증 절차 또는 실패 로그 부재와 관련된 정보성 항목으로 관리한다.

## 2. 결함 분류 체계

### 2.1 Severity 정의

| Severity | 정의 | 처리 기준 |
|---|---|---|
| Critical | 핵심 BMI 계산, 데이터 보정, 통계 산출이 중단되거나 잘못된 결과를 만들어 기능 출시를 막는 결함 | 즉시 수정, 병합 차단 |
| Major | 주요 요구사항이 일부 실패하거나 특정 조건에서 잘못된 결과가 발생하는 결함 | 현재 마일스톤 내 수정, 원칙적으로 병합 전 해결 |
| Minor | 우회 가능하거나 제한된 입력 조건에서만 발생하는 낮은 영향도의 결함 | 우선순위에 따라 수정, 릴리스 노트 또는 이슈로 추적 |
| Info | 제품 동작 결함은 아니지만 테스트 환경, 문서, 도구, 로그 품질 개선이 필요한 항목 | 필요 시 문서화 또는 작업 이슈로 전환 |

### 2.2 ItemType 정의

| ItemType | 정의 | 예시 |
|---|---|---|
| Functional | 요구사항의 공개 API 동작이 기대와 다른 결함 | `getBmiRatio`, 정상 BMI 사용자 목록, 전체 BMI 분포 API 결과 오류 |
| DataValidation | CSV 파싱, 잘못된 값, 누락값 보정 정책 처리 결함 | 비숫자 필드 예외, 음수 체중/키 처리, 보정 불가 레코드 포함 오류 |
| Boundary | BMI, 나이, 분모 0 등 경계값 처리 결함 | BMI `18.5`, `23.0`, 나이 `20`, `79`, `80`, 빈 데이터 |
| TestFailure | 자동화 테스트 실패 또는 기대값과 실제값 불일치 | Google Test 실패, Golden Master diff, flaky test |
| Tooling | 빌드, 테스트 실행, 커버리지, CI 환경 결함 | PowerShell 명령 호환성, `lcov` 미설치, CI 아티팩트 누락 |

### 2.3 Severity x ItemType 매트릭스

| Severity \ ItemType | Functional | DataValidation | Boundary | TestFailure | Tooling |
|---|---|---|---|---|---|
| Critical | BMI 계산 또는 전체 분포가 전면 오작동해 핵심 기능 사용 불가 | 유효 데이터가 대량 누락되거나 잘못 포함되어 통계 신뢰성 상실 | 핵심 경계값이 다수 오분류되어 요구사항 위반 | P0 테스트 또는 Golden Master가 지속 실패하고 제품 결함으로 확인 | 빌드/테스트가 전 환경에서 실행 불가 |
| Major | 특정 공개 API가 요구사항과 다르게 동작 | 누락값 보정, malformed row, 음수 값 처리 중 일부 정책 위반 | 주요 BMI/나이 경계값 하나 이상 오동작 | P0/P1 테스트 실패가 재현되며 수정 필요 | CI 또는 커버리지 잡 실패로 품질 게이트 확인 불가 |
| Minor | 제한된 입력 조합에서 결과 표시나 정렬이 기대와 다름 | 추가 컬럼, 빈 줄 등 낮은 위험 입력 처리 정책 불명확 | P2 특이 케이스 경계값 처리 미흡 | 테스트명, fixture, 기대값 가독성 문제 | 로컬 실행 스크립트, 리포트 경로 등 사용성 문제 |
| Info | 향후 요구사항 명확화 또는 API 명명 개선 제안 | 데이터 정책 문서화 필요 | 경계값 근거 주석 또는 문서 보강 필요 | 실패 로그 부재, 재현 불가 테스트 후보 | 셸 문법, 도구 설치 안내, 커버리지 산출물 정리 |

## 3. 결함 보고서 템플릿

결함은 아래 템플릿을 사용해 `docs/feature_defect_list.md` 또는 GitHub Issue에 등록한다.

```markdown
## [DEFECT-ID] 제목

- Severity:
- ItemType:
- 발견 단계:
- 관련 요구사항:
- 관련 테스트:
- 환경:

### 재현 절차

1.
2.
3.

### 기대 결과

-

### 실제 결과

-

### 원인 분석

-

### 수정 내용

-

### 검증 결과

- 실행 명령:
- 테스트 결과:
- 커버리지 영향:
- 회귀 확인:
```

필수 작성 기준은 다음과 같다.

| 항목 | 작성 기준 |
|---|---|
| 재현 | 입력 CSV, 테스트명, 실행 명령, OS/셸, 빌드 디렉터리를 포함한다. |
| 기대 | 요구사항 문서 또는 테스트 계획의 기준값을 인용한다. |
| 실제 | 실패 메시지, Golden Master diff, 반환값, 로그를 객관적으로 기록한다. |
| 원인 | 코드 위치, 입력 조건, 정책 누락, 환경 차이를 구분해 작성한다. |
| 수정 | 최소 수정 범위와 영향받는 API 또는 테스트를 기록한다. |
| 검증 | 통과한 테스트, 커버리지 변화, 재발 방지 테스트 ID를 남긴다. |

## 4. 품질 메트릭 수집 계획

### 4.1 테스트 통과율

| 메트릭 | 산식 | 수집 시점 | 목표 |
|---|---|---|---|
| 전체 테스트 통과율 | `통과 테스트 수 / 전체 테스트 수 * 100` | PR 생성, 수정 커밋, 병합 전 | 100% |
| P0 테스트 통과율 | `통과 P0 테스트 수 / 전체 P0 테스트 수 * 100` | 기능 구현 중 매 커밋 또는 PR 갱신 | 100% |
| 회귀 테스트 통과율 | `통과 회귀 테스트 수 / 전체 회귀 테스트 수 * 100` | 결함 수정 후 | 100% |
| Golden Master 통과 여부 | 승인 파일 diff 없음 | 주요 출력 변경 시 | Pass |

권장 실행 명령은 다음과 같다.

```powershell
cmake --build build; if ($LASTEXITCODE -eq 0) { ctest --test-dir build --output-on-failure }
```

PowerShell에서는 `&&` 대신 위와 같이 `$LASTEXITCODE`를 확인하는 방식을 사용한다.

### 4.2 커버리지

커버리지 목표는 `docs/feature_test_plan.md`의 기준을 따른다.

| 대상 | 목표 |
|---|---:|
| `src/main/cpp/SHealth.cpp` 라인 커버리지 | 90% 이상 |
| `src/main/cpp/SHealth.cpp` 브랜치 커버리지 | 85% 이상 |
| `StandardBmiClassifier.cpp` 라인 커버리지 | 95% 이상 |
| 신규 기능 코드 라인 커버리지 | 90% 이상 |
| 전체 `src/main/cpp` 라인 커버리지 | 90% 이상 |

GCC 또는 MinGW 환경에서는 별도 커버리지 빌드 디렉터리에서 `gcov`/`lcov`를 사용한다.

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

Windows 로컬에서 `lcov` 사용이 어렵다면 테스트 통과율은 Windows에서 확인하고, 커버리지 수치는 WSL 또는 Linux CI의 GCC 기반 결과를 기준으로 관리한다.

### 4.3 단계별 결함 발견율

결함 발견 단계는 다음처럼 구분한다.

| 단계 | 설명 | 주요 결함 유형 |
|---|---|---|
| 요구사항 분석 | 정책 불명확, 기준값 누락, API 반환 범위 모호성 확인 | Functional, Boundary, DataValidation |
| 테스트 설계 | 누락된 경계값, 예외 케이스, 회귀 시나리오 확인 | Boundary, TestFailure |
| 구현 및 단위 테스트 | Google Test 실행 중 기능 결함 확인 | Functional, DataValidation, TestFailure |
| 통합 검증 | Golden Master, 전체 CTest, 반복 호출 검증 | Functional, TestFailure |
| 커버리지 및 CI | 미검증 분기, 환경 문제, 품질 게이트 실패 확인 | Tooling, TestFailure |
| 릴리스 전 리뷰 | 잔여 위험, 문서 누락, 재현 불가 결함 정리 | Info, Minor |

단계별 결함 발견율은 다음 산식으로 수집한다.

```text
단계별 결함 발견율 = 해당 단계에서 최초 발견된 결함 수 / 전체 발견 결함 수 * 100
```

함께 수집할 보조 지표는 다음과 같다.

| 메트릭 | 산식 또는 기준 | 활용 |
|---|---|---|
| Severity 분포 | Severity별 결함 수 | 출시 위험도 판단 |
| ItemType 분포 | ItemType별 결함 수 | 취약 영역 식별 |
| 결함 제거율 | `수정 완료 결함 수 / 전체 결함 수 * 100` | 결함 관리 진행률 확인 |
| 재오픈율 | `재오픈 결함 수 / 수정 완료 결함 수 * 100` | 수정 품질 확인 |
| 결함 수정 리드타임 | `Closed 시각 - Opened 시각` | 병목 단계 파악 |
| 커버리지 미달 항목 수 | 목표 미달 파일 또는 모듈 수 | 추가 테스트 우선순위 결정 |

## 5. GitHub Issues 연동 워크플로우

GitHub Issues를 사용할 경우 결함 관리 흐름은 다음을 권장한다.

1. 결함 발견 시 이 문서의 템플릿으로 Issue를 생성한다.
2. `severity:critical`, `severity:major`, `severity:minor`, `severity:info` 중 하나를 지정한다.
3. `type:functional`, `type:data-validation`, `type:boundary`, `type:test-failure`, `type:tooling` 중 하나를 지정한다.
4. 관련 테스트 ID를 본문에 연결하고, 재현 가능한 경우 실패 로그 또는 Golden Master diff를 첨부한다.
5. 수정 PR 본문에 `Fixes #이슈번호`를 기록해 병합 시 자동 종료되도록 한다.
6. PR 검증에는 CTest 결과, 커버리지 결과, 회귀 테스트 추가 여부를 포함한다.

권장 라벨은 다음과 같다.

| 라벨 | 용도 |
|---|---|
| `severity:critical` | 병합 또는 릴리스를 차단하는 결함 |
| `severity:major` | 현재 마일스톤 내 해결해야 하는 주요 결함 |
| `severity:minor` | 우회 가능하거나 영향 범위가 제한된 결함 |
| `severity:info` | 문서, 도구, 재현성 개선 항목 |
| `type:functional` | 기능 요구사항 위반 |
| `type:data-validation` | 입력 검증 또는 누락값 보정 결함 |
| `type:boundary` | 경계값 처리 결함 |
| `type:test-failure` | 테스트 실패 또는 flaky test |
| `type:tooling` | 빌드, 커버리지, CI 환경 문제 |

Issue 상태는 `Open`, `In Progress`, `Fixed`, `Verified`, `Closed`, `Reopened`로 관리한다. `Fixed`는 코드 수정 완료 상태이고, `Verified`는 QA가 재현 절차와 회귀 테스트로 수정 효과를 확인한 상태로 구분한다.

## 6. 운영 기준

- `Critical`과 `Major` 결함은 원칙적으로 병합 전 해결한다.
- 제품 코드 결함이 아닌 `Info` 항목도 재현 명령과 환경을 기록해 같은 문제가 반복되지 않게 한다.
- 모든 결함 수정에는 최소 하나 이상의 재발 방지 테스트를 추가하거나, 테스트 추가가 불가능한 경우 사유를 기록한다.
- 커버리지 목표 미달 상태로 병합해야 한다면 미달 파일, 미검증 분기, 보완 계획을 PR에 명시한다.
- 결함 목록은 테스트 실행 결과와 함께 갱신하며, 재현 불가 항목은 마지막 확인 일자와 실행 명령을 남긴다.
