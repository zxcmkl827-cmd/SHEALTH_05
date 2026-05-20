# SHealth 결함 관리 보고서

## 1. 목적과 범위

본 문서는 SHealth BMI 계산 모듈의 결함 분류 기준, 결함 보고서 작성 템플릿, 품질 메트릭 수집 계획, GitHub Issues 연동 워크플로우를 정의한다.

대상 범위는 다음과 같다.

- `SHealth::calculateBmi(const std::string& filename)`
- `SHealth::getBmiRatio(int ageClass, int type)`
- `StandardBmiClassifier::classify(double bmi)`
- CSV 입력 파싱, 체중 누락값 보정, BMI 계산, 연령대별 BMI 범주 비율 산출 흐름
- Google Test, CTest, 커버리지 측정, CI 실행 결과

결함은 `docs/defect_list.md`의 `DEF-###` 형식을 기준으로 추적하며, 테스트 계획서의 P0/P1/P2 우선순위와 연결해 수정 우선순위를 판단한다.

## 2. 결함 분류 체계

### Severity 정의

| Severity | 정의 | 예시 |
|---|---|---|
| Critical | 프로그램 중단, 데이터 손상, 보안/안전 문제, 핵심 기능 전체 사용 불가 | 입력 CSV 처리 중 크래시, 배열 범위 초과로 메모리 오염, CI 전체 차단 |
| Major | 핵심 요구사항 위반 또는 주요 계산 결과 오류. 우회 가능하지만 릴리스 전 수정 필요 | BMI 25.0 미분류, 분모 0으로 NaN/Inf 발생, 잘못된 비율 산출 |
| Minor | 제한된 조건에서 발생하는 사용성/호환성/보조 기능 문제. 핵심 계산 흐름 영향은 낮음 | 오류 메시지 불명확, 일부 플랫폼 명령어 문법 차이, 추가 컬럼 정책 미정 |
| Info | 결함 후보, 개선 제안, 문서/절차 보완 항목 | 재현 로그 부족, 향후 CSV quoting 지원 검토, 메트릭 수집 자동화 제안 |

### ItemType 정의

| ItemType | 정의 | 대표 예시 |
|---|---|---|
| Functional | 요구사항 또는 사용자 관찰 동작과 실제 기능이 불일치하는 결함 | BMI 범주 분류 오류, 연령대 비율 계산 오류 |
| DataValidation | 입력 데이터 검증, 파싱, 경계값, 예외 처리 관련 결함 | 컬럼 누락, 비숫자 값, 음수 체중, 키 0 |
| TestFailure | 자동화 테스트 실패, flaky test, 테스트 기대값 오류 | Google Test 실패, CTest 실패, golden master mismatch |
| Tooling | 빌드, 커버리지, CI, 실행 환경, 명령어 호환성 문제 | PowerShell 명령어 문법 오류, gcov/lcov 환경 미구성 |
| Documentation | 요구사항, 테스트 계획, 결함 목록, README 불일치 또는 누락 | 요구사항 정책 미정, 결함 재현 절차 누락 |

### Severity x ItemType 매트릭스

| Severity \ ItemType | Functional | DataValidation | TestFailure | Tooling | Documentation |
|---|---|---|---|---|---|
| Critical | 핵심 BMI 계산이 전체적으로 불능이거나 잘못된 결과를 대량 산출 | 잘못된 입력으로 크래시, 메모리 오염, 무한 루프 발생 | 전체 테스트 실행 불가 또는 릴리스 차단 실패 | 빌드/CI가 전면 실패해 산출물 생성 불가 | 릴리스 차단 요구사항 누락으로 치명 결함 검증 불가 |
| Major | 특정 경계값 또는 주요 시나리오에서 요구사항 위반 | 컬럼 누락, 비숫자 값, 분모 0 등으로 핵심 결과 오류 | P0/P1 테스트 실패, 회귀 테스트 실패 | 주요 플랫폼에서 빌드 또는 커버리지 수집 실패 | 요구사항/테스트 계획과 구현 기준이 달라 QA 판단 오류 발생 |
| Minor | 제한된 입력 조합에서 결과 영향이 작거나 우회 가능 | 오류 메시지, 허용 범위, 추가 컬럼 등 정책성 문제 | P2 테스트 실패, 낮은 빈도의 flaky test | 로컬 명령어 호환성, 리포트 경로 문제 | 문서 오탈자, 예시 부족, 링크 누락 |
| Info | 개선 후보 또는 추가 분석 필요 | 정책 미정 입력에 대한 관찰 기록 | 테스트 보강 아이디어, 재현 로그 대기 | 도구 개선 제안, 자동화 후보 | 설명 보완, 추적성 개선 제안 |

## 3. 결함 보고서 템플릿

신규 결함은 다음 템플릿으로 등록한다. 한 결함은 하나의 관찰 가능한 문제만 다루며, 재현 명령과 입력 데이터를 가능한 한 작게 유지한다.

```markdown
## [DEF-###] <결함 제목>

- Severity: Critical | Major | Minor | Info
- ItemType: Functional | DataValidation | TestFailure | Tooling | Documentation
- Status: Open | In Progress | Resolved | Verified | Deferred | Rejected
- Priority: P0 | P1 | P2
- 발견 단계: 요구사항 분석 | 테스트 설계 | 단위 테스트 | 통합 테스트 | 회귀 테스트 | CI | 운영/사용자 제보
- 관련 요구사항/테스트: <문서 경로 또는 테스트 ID>
- 담당자: <이름>
- 발견일: YYYY-MM-DD
- 수정 버전/커밋: <커밋 또는 PR 링크>

### 재현 절차

1. <환경, 브랜치, 빌드 설정을 명시한다.>
2. <입력 파일 또는 테스트 데이터를 준비한다.>
3. <실행 명령을 작성한다.>

### 기대 결과

<요구사항, 테스트 계획, 사용자 관점에서 기대되는 정상 동작을 작성한다.>

### 실제 결과

<실제 출력, 실패 로그, 반환값, 스크린샷 위치, 커버리지 리포트 등 관찰 결과를 작성한다.>

### 원인 분석

<코드 위치, 데이터 조건, 분기 누락, 테스트 기대값 오류 등 확인된 원인을 작성한다. 원인 미확정이면 가설과 추가 확인 항목을 분리한다.>

### 수정 내용

<수정 요약, 영향 범위, 변경된 테스트, 문서 갱신 여부를 작성한다.>

### 검증 결과

<재실행한 테스트 명령, 통과/실패 결과, 커버리지 변화, 남은 리스크를 작성한다.>
```

## 4. 품질 메트릭 수집 계획

### 수집 대상 메트릭

| 메트릭 | 정의 | 산출 방식 | 목표/판단 기준 |
|---|---|---|---|
| 테스트 통과율 | 실행된 자동화 테스트 중 통과한 테스트 비율 | `passed / total * 100` | P0/P1 100%, 전체 테스트 100% Green |
| 라인 커버리지 | 실행된 코드 라인 비율 | 커버리지 도구 리포트 | 전체 90% 이상, `SHealth.cpp` 95% 이상 |
| 분기 커버리지 | 조건/분기 실행 비율 | 커버리지 도구 리포트 | 핵심 계산/검증 로직 85% 이상 |
| 단계별 결함 발견율 | 단계별 발견 결함 수의 비율 | `단계별 결함 수 / 전체 결함 수 * 100` | 요구사항/테스트 설계 단계 발견 비율을 높이고, CI/릴리스 후 발견 비율을 낮춘다 |
| 결함 재오픈율 | 수정 완료 후 재발 또는 검증 실패로 재오픈된 결함 비율 | `reopened / resolved * 100` | 5% 이하 |
| 평균 해결 시간 | Open부터 Resolved까지 걸린 평균 시간 | 이슈 생성/해결 timestamp 기준 | Critical 1영업일, Major 3영업일 이내를 우선 기준으로 관리 |

### 단계별 결함 발견율

결함 발견 단계는 다음 값 중 하나로 기록한다.

| 단계 | 설명 | 주요 산출물 |
|---|---|---|
| 요구사항 분석 | 요구사항 모호성, 누락, 상충 발견 | `docs/requirements_analysis.md` |
| 테스트 설계 | 테스트 케이스 작성 중 결함 후보 발견 | `docs/test_plan.md` |
| 단위 테스트 | Google Test, CTest 실행 중 결함 발견 | `src/test/cpp/SHealthBMITest.cpp` |
| 통합 테스트 | 실행 파일, 파일 입출력, 경로 설정 등 통합 흐름에서 발견 | `SHealthBMI`, 샘플 CSV |
| 회귀 테스트 | 기존 통과 기능이 변경 후 실패 | 회귀 테스트 결과 |
| CI | GitHub Actions 또는 원격 빌드에서 발견 | CI 로그 |
| 운영/사용자 제보 | 릴리스 후 사용자 또는 운영 환경에서 발견 | 이슈, 로그, 사용자 제보 |

월별 또는 릴리스별로 다음 형태로 집계한다.

| 기간 | 전체 결함 | 요구사항 분석 | 테스트 설계 | 단위 테스트 | 통합 테스트 | 회귀 테스트 | CI | 운영/사용자 제보 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| YYYY-MM | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

### C++: gcov/lcov

SHealth 현재 구현은 C++17, CMake, Google Test 기반이므로 C++ 커버리지는 GCC 또는 Clang 기반 별도 커버리지 빌드에서 수집한다.

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

수집 결과는 다음 기준으로 관리한다.

- `ctest` 통과율: 전체 테스트 100%
- 전체 라인 커버리지: 90% 이상
- `SHealth.cpp` 라인 커버리지: 95% 이상
- `StandardBmiClassifier` 분류 로직: 라인/분기 커버리지 100%
- 커버리지 제외 대상: 외부 의존성 `_deps`, 테스트 코드, 시스템 헤더

### Java: JaCoCo

Java 모듈이 추가될 경우 JaCoCo를 표준 커버리지 도구로 사용한다.

| 빌드 도구 | 권장 명령 | 산출물 |
|---|---|---|
| Maven | `mvn clean test jacoco:report` | `target/site/jacoco/index.html` |
| Gradle | `./gradlew test jacocoTestReport` | `build/reports/jacoco/test/html/index.html` |

권장 기준은 라인 커버리지 90% 이상, 핵심 서비스/도메인 로직 분기 커버리지 85% 이상이다. 테스트 코드, generated source, DTO 단순 getter/setter는 필요 시 제외 규칙을 명시한다.

### Python: pytest-cov

Python 스크립트나 데이터 검증 도구가 추가될 경우 `pytest-cov`를 사용한다.

```bash
python -m pytest --cov=. --cov-report=term-missing --cov-report=html
```

권장 기준은 라인 커버리지 90% 이상, 핵심 데이터 파싱/검증 로직 95% 이상이다. HTML 리포트는 기본적으로 `htmlcov/index.html`에서 확인한다.

## 5. GitHub Issues 연동 워크플로우

GitHub Issues를 사용할 경우 결함 관리는 다음 흐름을 따른다.

1. 결함 발견 시 `bug` 라벨로 Issue를 생성하고, 제목은 `[DEF-###] <요약>` 형식을 사용한다.
2. Issue 본문은 본 문서의 결함 보고서 템플릿을 사용한다.
3. Severity와 ItemType은 라벨로 함께 부여한다. 예: `severity:major`, `type:data-validation`.
4. 테스트 실패 결함은 실패한 테스트명, 실행 명령, 최소 입력 데이터를 반드시 첨부한다.
5. 수정 PR은 Issue를 연결한다. 예: `Fixes #12`.
6. PR에서는 수정 코드와 함께 회귀 테스트 또는 검증 절차를 포함한다.
7. CI Green, 리뷰 승인, 결함 템플릿의 검증 결과 작성 후 Issue를 닫는다.

권장 라벨은 다음과 같다.

| 라벨 그룹 | 값 |
|---|---|
| Severity | `severity:critical`, `severity:major`, `severity:minor`, `severity:info` |
| ItemType | `type:functional`, `type:data-validation`, `type:test-failure`, `type:tooling`, `type:documentation` |
| Status | `status:triage`, `status:in-progress`, `status:blocked`, `status:verified` |
| Priority | `priority:p0`, `priority:p1`, `priority:p2` |

Issue 상태 전이는 다음을 기본으로 한다.

```text
Open -> Triage -> In Progress -> Resolved -> Verified -> Closed
                    \-> Blocked
                    \-> Deferred
                    \-> Rejected
```

## 6. 운영 기준

- Critical 결함은 즉시 triage하고, 재현 가능한 최소 케이스를 확보한 뒤 수정 전까지 릴리스 차단 항목으로 관리한다.
- Major 결함은 P0/P1 테스트와 연결해 릴리스 전 수정 여부를 판단한다.
- Minor 결함은 릴리스 리스크와 수정 비용을 비교해 스프린트 내 처리 또는 backlog로 분류한다.
- Info 항목은 결함 후보와 개선 제안을 구분하고, 재현 로그 확보 전까지 제품 결함으로 확정하지 않는다.
- 모든 수정 결함은 최소 하나 이상의 자동화 테스트 또는 명시적 수동 검증 기록을 남긴다.
- 결함 목록, 테스트 계획, 요구사항 분석 문서가 서로 다른 정책을 말하면 요구사항 분석 문서를 먼저 갱신한 뒤 테스트와 결함 기준을 맞춘다.
