# SHealth 코드 품질 분석 보고서

## 1. 분석 범위

본 문서는 `src/main/cpp/SHealth.cpp`에 정의된 메소드들을 SOLID 원칙과 Code Smell 관점에서 분석한다. 참고 범위에는 공개 API 선언부인 `SHealth.h`와 실행 출력 예시인 `SHealthBMI.cpp`를 포함한다.

우선순위는 `1`이 가장 높고 `5`가 가장 낮다.

## 2. 문제점 분석

| 문제점 | 위반 원칙/스멜 | 영향 | 개선 방향 | 우선순위 |
|---|---|---|---|---:|
| `SHealth`가 파일 읽기, CSV 파싱, 데이터 보정, BMI 계산, BMI 분류 위임, 통계 집계, 조회 API까지 모두 조율한다. | SRP 위반, God Class 경향 | 기능 추가 시 `SHealth.cpp`가 계속 비대해지고, CSV 형식 변경이나 통계 정책 변경이 같은 클래스 수정으로 이어진다. 단위 테스트도 public API 중심의 통합 테스트에 치우친다. | `CsvHealthRecordReader`, `MissingValueImputer`, `BmiCalculator`, `BmiStatisticsService` 등 역할 단위 클래스로 분리하고 `SHealth`는 facade 또는 application service로 축소한다. | 1 |
| `calculateBmi()`가 상태 초기화, 파일 로딩, 결측치 보정, BMI 계산, 집계 저장을 순서대로 직접 수행한다. | SRP 위반, Long Method 가능성, Temporal Coupling | 현재 길이는 짧지만 처리 파이프라인의 순서 의존성이 한 메소드에 감춰져 있다. 향후 height 보정, 정상 BMI 사용자 조회, 전체 비율 기능이 추가되면 빠르게 긴 메소드가 된다. | `load -> normalize/impute -> calculate -> aggregate` 파이프라인을 별도 서비스 또는 순수 함수 조합으로 분리한다. 각 단계의 입력/출력을 명확히 해 테스트 단위를 낮춘다. | 1 |
| `loadRecords()`, `parseRecord()`, `split()`가 `SHealth` 내부 private 메소드로 묶여 있다. | SRP 위반, 낮은 재사용성 | CSV 헤더 처리, 컬럼 순서, 구분자, malformed row 정책이 BMI 도메인 서비스와 결합된다. 다른 입력 소스나 테스트 fixture 재사용이 어렵다. | `HealthRecordReader` 인터페이스와 `CsvHealthRecordReader` 구현체로 분리한다. C++17에서는 `std::optional<HealthRecord>` 파서와 `std::vector<HealthRecord>` reader를 독립 테스트 대상으로 둔다. | 1 |
| `parseRecord()`가 ID 컬럼을 읽지 않고 age, weight, height만 보존한다. | SRP/요구사항 누락 위험, Primitive Obsession | 정상 BMI 사용자 목록 조회 같은 기능에서 원본 사용자 식별자를 되살릴 수 없다. CSV 스키마 지식이 인덱스 상수에만 남아 도메인 모델이 불완전하다. | `HealthRecord`를 독립 모델로 분리하고 `id`, `age`, `weight`, `height`, `bmi`를 명시한다. 컬럼 인덱스는 parser 내부 상수 또는 스키마 객체로 제한한다. | 2 |
| 결측치 보정이 `imputeMissingWeights()`에만 존재하고, 평균 계산도 체중 전용이다. | OCP 위반, Duplicated Code 잠재 | height 보정 요구가 추가되면 `calculateAverageHeights()`, `imputeMissingHeights()`가 거의 같은 구조로 복제될 가능성이 높다. | 값 접근자와 수정자를 받는 generic imputer, 또는 `MissingMetric { Weight, Height }` 기반 정책 클래스로 분리한다. C++17에서는 람다와 템플릿 함수로 중복을 줄일 수 있다. | 2 |
| `calculateAverageWeights()`와 `calculateRatioSet()`가 모두 records를 순회하며 `toAgeClass()`로 그룹핑한다. | Duplicated Code, 분산된 그룹핑 정책 | 연령대 기준이 바뀌면 여러 집계 루프를 동시에 고쳐야 한다. 대량 데이터에서는 불필요한 반복 순회도 늘어난다. | `AgeClassPolicy` 또는 `groupByAgeClass()` 헬퍼를 도입한다. 중간 집계 구조를 만든 뒤 평균, 비율 등 통계 계산이 같은 그룹 데이터를 공유하게 한다. | 2 |
| `AgeClasses`, `MinAgeClass`, `MaxAgeClass`, `AgeClassStep`가 별도로 존재해 지원 연령대 정책이 중복 표현된다. | Magic Number/Shotgun Surgery 가능성 | `20~70`, `10년 단위`, `80 미만` 정책이 배열과 범위 계산식에 나뉘어 있다. 하나만 수정하면 집계 대상과 유효성 판단이 어긋날 수 있다. | 단일 `AgeClassPolicy`에서 `classes()`, `toAgeClass(age)`, `contains(ageClass)`를 제공한다. 배열은 `Min/Max/Step`로 생성하거나 반대로 배열 기준으로 유효성을 판단한다. | 2 |
| BMI 타입 외부 코드 `100/200/300/400`을 `categoryFromType()`의 `switch`로 변환한다. | OCP 위반, Switch Statement, Primitive Obsession | 범주 추가나 외부 코드 변경 시 switch와 출력 호출부를 함께 수정해야 한다. `getBmiRatio(int, int)` 호출자는 유효한 type을 컴파일 타임에 보장받지 못한다. | public API는 `BmiCategory` 또는 `enum class BmiTypeCode`를 받도록 확장하고, 기존 int API는 compatibility adapter로 격리한다. 변환은 `std::array<std::pair<int, BmiCategory>, N>` 테이블 기반으로 바꿀 수 있다. | 2 |
| `RatioSet`이 `std::array<double, 4>`로 고정되어 `BmiCategory`의 개수와 암묵적으로 결합된다. | Magic Number, Parallel Structure | 카테고리 수가 바뀌면 `RatioSet`, counts 배열, 변환 로직을 함께 수정해야 한다. `static_cast<std::size_t>(category)`에 enum 순서 의존성이 생긴다. | `constexpr std::array<BmiCategory, 4>` 또는 `std::map<BmiCategory, double>`를 사용한다. 성능이 중요하면 enum count 상수와 `toIndex(BmiCategory)`를 한 곳에 둔다. | 3 |
| `parseRecord()`가 `catch (...)`로 모든 예외를 삼킨다. | Error Handling Smell, 숨겨진 실패 | 잘못된 숫자 입력과 예상치 못한 예외가 같은 스킵으로 처리되어 원인 추적이 어렵다. 데이터 품질 리포팅이나 테스트 진단 정보가 부족하다. | `std::invalid_argument`, `std::out_of_range`만 명시적으로 처리한다. 필요하면 `ParseResult`에 실패 사유를 담거나 로깅 가능한 진단 정보를 분리한다. | 3 |
| `record.weight == 0.0`, `record.height <= 0.0`, `record.bmi <= 0.0`, `total == 0` 같은 sentinel 값 정책이 여러 메소드에 흩어져 있다. | Magic Number, Primitive Obsession, 조건문 분산 | `0`이 누락값인지 유효하지 않은 값인지 계산 제외 조건인지 문맥마다 다르다. height `0` 보정 요구처럼 정책이 바뀌면 조건 누락 위험이 크다. | `std::optional<double>`로 누락값을 표현하거나 `HealthRecord`에 `hasValidWeight()`, `hasValidHeight()`, `hasValidBmi()` 같은 도메인 함수를 둔다. | 3 |
| `calculateRatioSet()`가 연령대 필터링, BMI 유효성 필터링, 분류, 카운트, 퍼센트 변환을 한 루프와 후속 루프에서 처리한다. | 조건문 복잡도, Mixed Level of Abstraction | 조건은 단순하지만 여러 추상화 수준이 섞여 있어 전체/연령대별 통계 추가 시 분기가 늘어날 가능성이 높다. | `recordsForAgeClass()`, `countByCategory()`, `toPercentages()` 같은 순수 함수로 나눈다. 또는 `BmiStatisticsService`가 범위 조건을 predicate로 받아 집계하도록 한다. | 3 |
| `SHealthBMI.cpp`가 20~70대와 100~400 타입 조합을 `printf`로 반복 호출한다. | Duplicated Code, Magic Number, OCP 위반 | 출력 대상 연령대나 BMI 범주가 바뀌면 6개의 거의 같은 문장을 수정해야 한다. 라이브러리의 상수 정책과 실행 출력 코드가 따로 움직인다. | `AgeClassPolicy::classes()`와 BMI category/code 테이블을 순회해 출력한다. 가능하면 `std::cout`과 range 기반 for를 사용해 C++ 스타일로 정리한다. | 4 |
| `percentage()` 내부의 `100.0`, `CentimetersPerMeter`, CSV delimiter, 컬럼 인덱스 등은 상수화되어 있으나 일부는 도메인 의미가 더 드러나야 한다. | Magic Number 관리 미흡 | 단순 상수화는 되어 있지만 `PercentageScale`, `InputHeightUnit`, `CsvSchema`처럼 정책의 이름이 충분히 드러나지 않는다. 유지보수자는 숫자의 단위와 적용 범위를 코드 주변에서 추론해야 한다. | 기존 익명 namespace 상수는 유지하되, 성격별로 `CsvSchema`, `AgeClassPolicy`, `MeasurementUnit`에 묶는다. `100.0`은 `PercentageScale`로 이름을 부여한다. | 4 |
| `getBmiRatio()`가 계산 전 호출, 파일 로딩 실패 후 호출, 유효하지 않은 age/type 호출을 모두 `0.0`으로 반환한다. | Ambiguous Return Value, 오류 표현 부족 | 실제 비율 0%와 잘못된 요청 또는 미계산 상태를 구분할 수 없다. 호출자 입장에서 데이터 없음과 오류가 동일하게 보인다. | 새 API에서는 `std::optional<double>` 또는 결과 객체를 반환한다. 기존 API는 하위 호환용으로 유지하되 내부적으로 명확한 상태를 갖는 API에 위임한다. | 4 |
| `BmiClassifier` 전략은 도입되어 있으나 나머지 정책은 여전히 `SHealth` 내부 조건과 상수에 고정되어 있다. | OCP 부분 위반 | BMI 분류 기준은 교체 가능하지만 연령대 정책, 입력 스키마, 결측치 정책, 통계 산식은 교체하기 어렵다. | 전략 패턴을 `BmiClassifier`뿐 아니라 `AgeClassPolicy`, `MissingValuePolicy`, `RecordReader`로 확장한다. 과도한 인터페이스 분리는 피하고 변경 가능성이 높은 정책부터 적용한다. | 5 |

## 3. C++17 개선 방향 요약

### 3.1 전략 패턴 적용 후보

| 전략 후보 | 목적 | 적용 효과 |
|---|---|---|
| `BmiClassifier` | BMI 경계값별 범주 결정 | 이미 적용된 구조를 유지하고 경계값 테스트를 강화한다. |
| `HealthRecordReader` | 파일/CSV 입력 소스 분리 | CSV 스키마 변경이나 테스트용 메모리 입력을 `SHealth` 수정 없이 교체한다. |
| `MissingValuePolicy` | 체중/키 결측치 보정 정책 분리 | 평균 보정, 스킵, 기본값 대입 같은 정책을 독립적으로 테스트한다. |
| `AgeClassPolicy` | 연령대 산정 기준 분리 | 20~70대, 10년 단위 같은 정책 변경을 한 곳으로 제한한다. |
| `BmiStatisticsService` | 집계 방식 분리 | 연령대별 비율, 전체 비율, 정상 BMI 목록 조회를 같은 집계 기반 위에서 확장한다. |

### 3.2 테이블 기반 개선 후보

`categoryFromType()`의 `switch`와 `SHealthBMI.cpp`의 반복 출력은 테이블 기반 구조로 바꾸는 것이 적합하다.

```cpp
constexpr std::array<std::pair<int, BmiCategory>, 4> BmiTypeMappings{{
    {100, BmiCategory::Underweight},
    {200, BmiCategory::Normal},
    {300, BmiCategory::Overweight},
    {400, BmiCategory::Obesity},
}};
```

이 방식은 BMI 타입 코드와 범주의 관계를 한 곳에 모으고, 조회와 출력 양쪽에서 같은 테이블을 순회할 수 있게 한다.

### 3.3 `std::variant` 적용 가능성

현재 구조에서는 `std::variant`가 최우선 도구는 아니다. 다만 다음 요구가 커지면 적용 가치가 있다.

| 적용 후보 | 예시 | 판단 |
|---|---|---|
| 입력 소스 | `std::variant<CsvFileInput, InMemoryInput>` | 입력 타입이 2~3개로 고정되고 런타임 다형성이 부담스러울 때 적합하다. |
| 파싱 결과 | `std::variant<HealthRecord, ParseError>` | 실패 사유를 보존하면서 예외 없는 파싱 흐름을 만들 때 유용하다. |
| 통계 요청 | `std::variant<AgeClassDistributionRequest, OverallDistributionRequest, NormalUsersRequest>` | 조회 API가 늘어나고 요청별 처리를 visitor로 명확히 나누고 싶을 때 고려한다. |

현재 리팩토링 1차 목표는 `variant`보다 책임 분리, 테이블 기반 매핑, 명시적 도메인 타입 도입이 더 효과적이다.

## 4. 리팩토링 우선순위

| 우선순위 | 작업 | 이유 |
|---:|---|---|
| 1 | CSV 읽기/파싱과 `SHealth` 도메인 흐름 분리 | SRP 위반의 중심이며 테스트 단위 개선 효과가 가장 크다. |
| 2 | 결측치 보정과 연령대 그룹핑 정책 분리 | height 보정과 전체 통계 기능 추가 시 중복과 조건 누락을 줄인다. |
| 3 | BMI type int API를 enum/table 기반으로 격리 | OCP 위반과 magic number 노출을 줄이고 호출 안정성을 높인다. |
| 4 | 통계 집계를 `BmiStatisticsService`로 분리 | 연령대별 비율, 전체 비율, 정상 BMI 목록 조회를 같은 구조로 확장할 수 있다. |
| 5 | `SHealthBMI.cpp` 출력 반복 제거 | 사용자-facing 실행 예시는 깔끔해지지만 핵심 도메인 안정성에 비해 영향이 작다. |

## 5. 결론

현재 `SHealth.cpp`는 이전보다 매직 넘버 상수화와 `BmiClassifier` 전략 분리가 일부 적용되어 있어 리팩토링 출발점은 좋다. 그러나 핵심 문제는 `SHealth`가 여전히 입력, 보정, 계산, 집계, 조회를 모두 소유한다는 점이다.

가장 먼저 CSV 파싱과 결측치/연령대 정책을 분리하면 SRP 위반과 중복 가능성을 동시에 줄일 수 있다. 이후 BMI 타입 매핑을 enum/table 기반으로 정리하고 통계 서비스를 분리하면 신규 기능을 `SHealth.cpp` 수정 없이 추가하는 OCP 친화 구조에 가까워진다.
