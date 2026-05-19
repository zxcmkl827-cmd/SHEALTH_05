# SHealth Code Quality Report

## 분석 범위

- 대상 파일: `src/main/cpp/SHealth.h`, `src/main/cpp/SHealth.cpp`
- 대상 메서드: `SHealth::calculateBmi`, `SHealth::getBmiRatio`, `SHealth::split`
- 관점: SOLID, Code Smell, Magic Number, C++17 리팩토링 가능성
- 우선순위 기준: `1`이 가장 높고, `5`가 가장 낮음

## 문제점 분석

| 문제점 | 위반 원칙/스멜 | 영향 | 개선 방향 | 우선순위 |
|---|---|---|---|---|
| `calculateBmi`가 파일 열기, CSV 파싱, 데이터 저장, 결측 체중 보정, BMI 계산, 나이대별 통계 집계를 모두 수행함 | SRP 위반, Long Method | 메서드 변경 이유가 과도하게 많아지고, 파일 형식 변경이나 BMI 기준 변경이 동일 메서드 수정을 유발함. 테스트도 단계별로 분리하기 어려움 | `loadRecords`, `imputeMissingWeights`, `calculateBmiValues`, `aggregateRatios`처럼 책임 단위 함수로 분리. 파일 I/O와 도메인 계산을 분리해 순수 계산 로직을 독립 테스트 가능하게 구성 | 1 |
| BMI 분류 기준 `18.5`, `23`, `25`가 조건문 안에 직접 사용됨 | Magic Number, OCP 위반 가능성 | 기준 변경 시 조건문 전체를 찾아 수정해야 하며, 경계값 의미가 코드만으로 명확하지 않음. `bmis[i] == 25`가 어떤 분류에도 포함되지 않는 버그 가능성이 있음 | `constexpr double UnderweightMax = 18.5;`, `NormalMax = 23.0;`, `OverweightMax = 25.0;` 또는 `BmiRange` 테이블로 상수화. 경계 포함 규칙을 한 곳에서 명시 | 1 |
| 나이대 조건 `20`, `70`, `10`, `a + 10`이 반복됨 | Magic Number, Duplicated Code | 지원 나이대가 변경되면 여러 루프와 조건을 함께 수정해야 하며, 나이대 정의가 코드 흐름에 흩어짐 | `constexpr int MinAgeClass = 20;`, `MaxAgeClass = 70;`, `AgeClassStep = 10;` 또는 `std::array<int, 6>`로 나이대 목록을 정의. `ageToClass(age)` 헬퍼로 구간 판정 캡슐화 | 2 |
| `calculateBmi`의 결과 저장이 `if (a == 20) ... else if ...` 체인으로 나이대별 멤버 변수에 직접 매핑됨 | OCP 위반, Duplicated Code, 조건문 복잡도 | 새 나이대나 새 BMI 분류가 추가될 때 멤버 변수와 조건 체인이 동시에 증가함. 데이터 구조가 도메인 모델 대신 변수명 조합에 의존함 | `std::map<int, RatioSet>` 또는 `std::array<RatioSet, AgeClassCount>`로 나이대별 결과 저장. `RatioSet{underweight, normal, overweight, obesity}` 구조체를 도입 | 1 |
| `getBmiRatio`가 `ageClass`와 `type` 조합을 24개의 `if/else if`로 조회함 | OCP 위반, Long Conditional, Duplicated Code | 조회 조건이 늘어날수록 메서드가 선형 증가하고, 잘못된 `type` 값이 컴파일 타임에 드러나지 않음 | `enum class BmiCategory { Underweight, Normal, Overweight, Obesity };`와 `std::map<int, RatioSet>` 기반 조회로 변경. 외부 API 호환이 필요하면 `type`을 enum으로 변환하는 어댑터만 유지 | 1 |
| BMI 타입 코드 `100`, `200`, `300`, `400`이 의미 없이 노출됨 | Magic Number, Primitive Obsession | 호출자가 숫자 코드 의미를 알아야 하며, 잘못된 값 전달을 방지하기 어렵다. 타입 추가나 변경 시 조건문 수정 범위가 커짐 | `enum class BmiCategoryCode` 또는 `enum class BmiCategory`로 의미를 부여. 기존 정수 API가 필요하면 `toBmiCategory(int type)` 변환 함수에서만 숫자 코드를 관리 | 2 |
| `ages`, `heights`, `weights`, `bmis`가 고정 크기 C 배열 `10000`으로 관리됨 | Magic Number, Primitive Obsession, 데이터 응집도 부족 | 입력 데이터가 10000건을 넘으면 범위 초과 위험이 있고, 한 사람의 데이터가 여러 배열에 분산되어 응집도가 낮음 | `struct HealthRecord { int age; double height; double weight; double bmi; };`와 `std::vector<HealthRecord>` 사용. 최대 건수 제한이 요구사항이면 `constexpr std::size_t MaxRecords`로 명시하고 검증 | 2 |
| CSV 토큰 접근 시 `tokens[1]`, `tokens[2]`, `tokens[3]`을 검증 없이 사용함 | Magic Number, Robustness Smell | 빈 줄이 아니지만 컬럼 수가 부족한 입력에서 예외나 범위 오류가 발생할 수 있음. 컬럼 의미가 인덱스만으로 드러나지 않음 | `constexpr std::size_t AgeColumn = 1;` 등 컬럼 상수 정의. `parseRecord`에서 컬럼 수와 변환 예외를 처리하고, 유효 레코드만 반환 | 2 |
| 결측 체중 보정에서 `sum / ageCount` 수행 전 `ageCount == 0` 검증이 없음 | 잠재 런타임 버그, Defensive Gap | 특정 나이대에 유효 체중 데이터가 없으면 0으로 나누는 문제가 발생할 수 있음 | 나이대별 평균 계산 결과를 `std::optional<double>`로 표현. 평균이 없을 때 레코드를 제외하거나 정책 기반으로 처리 | 1 |
| 비율 계산에서 `sum == 0` 검증 없이 `count * 100 / sum`을 수행함 | 잠재 런타임 버그, Defensive Gap | 특정 나이대 데이터가 없으면 0으로 나누는 문제가 발생함 | `sum == 0`이면 해당 `RatioSet`을 0 또는 `std::optional<RatioSet>`으로 저장. 정책을 명시해 호출자에게 빈 통계와 0%를 구분할 수 있게 함 | 1 |
| `calculateBmi`가 `std::cerr`에 직접 에러를 출력하고 `0`을 반환함 | SRP 위반, 오류 처리 Smell | 라이브러리 성격의 클래스가 출력 정책을 가지며, 파일 열기 실패와 데이터 0건을 구분하기 어렵다 | `std::optional<std::size_t>`, `expected` 유사 타입, 예외, 또는 오류 코드 객체로 실패를 표현. 로깅/출력은 호출자 계층에서 담당 | 3 |
| `split`은 단순 구분자 분리만 수행하며 CSV의 따옴표, 이스케이프, 빈 필드 정책을 다루지 않음 | Primitive Parsing, Hidden Assumption | 입력 CSV가 조금만 복잡해져도 파싱 결과가 깨질 수 있음 | 요구사항이 단순 CSV이면 `parseRecord` 내부 구현으로 숨기고, 표준 CSV가 필요하면 검증된 CSV 파서 또는 명확한 파싱 정책 도입 | 4 |
| BMI 분류와 나이대별 집계 정책이 코드에 하드코딩되어 있음 | OCP 위반 | 정책 변경 시 기존 계산 로직 수정이 필요하며, 국가/기관별 BMI 기준 같은 변형을 수용하기 어렵다 | 전략 패턴으로 `BmiClassifier` 인터페이스를 두거나, `std::array<BmiRule, N>` 테이블 기반 분류를 사용. 단순 정책이면 테이블 기반이 전략 패턴보다 가볍다 | 3 |

## C++17 개선 방향 요약

1. `HealthRecord`, `RatioSet`, `BmiRule` 같은 값 타입을 먼저 도입해 병렬 배열과 멤버 변수 조합을 제거한다.
2. BMI 기준, 나이대 목록, CSV 컬럼 인덱스, 타입 코드는 `constexpr`, `enum class`, `std::array`로 상수화한다.
3. `calculateBmi`를 파일 로딩, 레코드 파싱, 결측치 보정, BMI 계산, 비율 집계 단계로 분리한다.
4. `getBmiRatio`는 조건문 체인 대신 `std::map<int, RatioSet>` 또는 고정 나이대라면 `std::array<RatioSet, AgeClassCount>` 조회로 바꾼다.
5. BMI 분류는 현재처럼 단순한 구간 규칙이면 테이블 기반이 적합하고, 사용자/기관별 정책 교체가 요구되면 전략 패턴을 적용한다.
6. `std::variant`는 파싱 결과처럼 `ValidRecord`, `InvalidRecord`를 구분하거나, 향후 다양한 입력 레코드 타입을 처리해야 할 때 고려할 수 있다. 현재 BMI 분류 자체에는 `enum class`와 테이블 기반 구조가 더 단순하다.

## 리팩토링 우선순위

| 우선순위 | 작업 | 이유 |
|---|---|---|
| 1 | 0 나누기 가능성 제거, BMI 경계값 상수화, `calculateBmi` 책임 분리 | 런타임 오류 가능성과 핵심 도메인 규칙의 불명확성을 먼저 해결해야 함 |
| 2 | 병렬 배열과 나이대/타입 조건 체인을 구조체, enum, 컨테이너로 대체 | 변경에 취약한 데이터 표현을 개선하면 이후 수정 비용이 크게 줄어듦 |
| 3 | 파일 I/O와 오류 처리 정책 분리 | 테스트성과 재사용성을 높이고 호출자에게 실패 원인을 명확히 전달할 수 있음 |
| 4 | BMI 분류 정책을 테이블 기반 또는 전략 패턴으로 외부화 | 정책 변경 가능성이 커질 때 OCP를 만족하는 확장 구조를 제공 |
| 5 | CSV 파서 개선 또는 외부 파서 도입 검토 | 입력 형식 요구사항이 복잡해질 때 적용하면 충분하며, 현재 핵심 위험보다는 낮음 |
