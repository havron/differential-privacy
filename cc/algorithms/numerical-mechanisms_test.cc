//
// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "algorithms/numerical-mechanisms.h"

#include <vector>

#include "base/testing/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "base/statusor.h"
#include "algorithms/distributions.h"

namespace differential_privacy {
namespace {

using ::testing::_;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::Return;
using ::differential_privacy::base::testing::StatusIs;

constexpr int kSmallNumSamples = 1000000;

class MockLaplaceDistribution : public internal::LaplaceDistribution {
 public:
  MockLaplaceDistribution() : internal::LaplaceDistribution(1.0, 1.0) {}
  MOCK_METHOD1(Sample, double(double));
};

template <typename T>
class NumericalMechanismsTest : public ::testing::Test {};

typedef ::testing::Types<int64_t, double> NumericTypes;
TYPED_TEST_SUITE(NumericalMechanismsTest, NumericTypes);

TYPED_TEST(NumericalMechanismsTest, LaplaceBuilder) {
  LaplaceMechanism::Builder test_builder;
  auto test_mechanism = test_builder.SetL1Sensitivity(3).SetEpsilon(1).Build();
  ASSERT_OK(test_mechanism);

  EXPECT_DOUBLE_EQ((*test_mechanism)->GetEpsilon(), 1);
  EXPECT_DOUBLE_EQ(
      dynamic_cast<LaplaceMechanism*>(test_mechanism->get())->GetSensitivity(),
      3);
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsEpsilonNotSet) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL1Sensitivity(1).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Epsilon has to be set.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsEpsilonZero) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL1Sensitivity(1).SetEpsilon(0).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Epsilon has to be positive.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsEpsilonNegative) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL1Sensitivity(1).SetEpsilon(-1).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Epsilon has to be positive.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsEpsilonNan) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL1Sensitivity(1).SetEpsilon(NAN).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Epsilon has to be finite.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsEpsilonInfinity) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build =
      test_builder.SetL1Sensitivity(1).SetEpsilon(INFINITY).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Epsilon has to be finite.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsL0SensitivityNan) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(NAN)
                          .SetLInfSensitivity(1)
                          .SetEpsilon(1)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^L0 sensitivity has to be finite.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsL0SensitivityInfinity) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(INFINITY)
                          .SetLInfSensitivity(1)
                          .SetEpsilon(1)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^L0 sensitivity has to be finite.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsLInfSensitivityNan) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(1)
                          .SetLInfSensitivity(NAN)
                          .SetEpsilon(1)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^LInf sensitivity has to be finite.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsL0SensitivityNegative) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(-1)
                          .SetLInfSensitivity(1)
                          .SetEpsilon(1)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message,
              MatchesRegex("^L0 sensitivity has to be positive but is.*"));
}

TEST(NumericalMechanismsTest, LaplaceBuilderFailsLInfSensitivityZero) {
  LaplaceMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(1)
                          .SetLInfSensitivity(0)
                          .SetEpsilon(1)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message,
              MatchesRegex("^LInf sensitivity has to be positive but is.*"));
}

TYPED_TEST(NumericalMechanismsTest, LaplaceBuilderSensitivityTooHigh) {
  LaplaceMechanism::Builder test_builder;
  base::StatusOr<std::unique_ptr<NumericalMechanism>> test_mechanism =
      test_builder.SetL1Sensitivity(std::numeric_limits<double>::max())
          .SetEpsilon(1)
          .Build();
  EXPECT_FALSE(test_mechanism.ok());
}

TEST(NumericalMechanismsTest, LaplaceAddsNoise) {
  auto distro = absl::make_unique<MockLaplaceDistribution>();
  ON_CALL(*distro, Sample(_)).WillByDefault(Return(10.0));
  LaplaceMechanism mechanism(1.0, 1.0, std::move(distro));

  EXPECT_THAT(mechanism.AddNoise(0.0), DoubleNear(10.0, 5.0));
}

TEST(NumericalMechanismsTest, LaplaceAddsNoNoiseWhenSensitivityIsZero) {
  LaplaceMechanism mechanism(1.0, 0.0);

  EXPECT_THAT(mechanism.AddNoise(12.3), DoubleEq(12.3));
}

TEST(NumericalMechanismsTest, LaplaceNoisedValueAboveThreshold) {
  LaplaceMechanism::Builder builder;
  std::unique_ptr<NumericalMechanism> mechanism =
      builder.SetL1Sensitivity(1).SetEpsilon(1).Build().ValueOrDie();

  struct TestScenario {
    double input;
    double threshold;
    double expected_probability;
  };

  // To reduce flakiness from randomness, perform multiple trials and declare
  // the test successful if a sufficient expected number of trials provide the
  // expected result.
  std::vector<TestScenario> test_scenarios = {
      {-0.5, -0.5, 0.5000}, {0.0, -0.5, 0.6967}, {0.5, -0.5, 0.8160},
      {-0.5,  0.0, 0.3035}, {0.0,  0.0, 0.5000}, {0.5,  0.0, 0.6967},
      {-0.5,  0.5, 0.1840}, {0.0,  0.5, 0.3035}, {0.5,  0.5, 0.5000},
  };

  double num_above_thresold;
  for (TestScenario ts : test_scenarios) {
    num_above_thresold = 0;
    for (int i = 0; i < kSmallNumSamples; ++i) {
      if (mechanism->NoisedValueAboveThreshold(ts.input, ts.threshold))
        ++num_above_thresold;
    }
    EXPECT_NEAR(num_above_thresold / kSmallNumSamples, ts.expected_probability,
                0.0025);
  }
}

TEST(NumericalMechanismsTest, LaplaceDiversityCorrect) {
  LaplaceMechanism mechanism(1.0, 1.0);
  EXPECT_EQ(mechanism.GetDiversity(), 1.0);

  LaplaceMechanism mechanism2(2.0, 1.0);
  EXPECT_EQ(mechanism2.GetDiversity(), 0.5);

  LaplaceMechanism mechanism3(2.0, 3.0);
  EXPECT_EQ(mechanism3.GetDiversity(), 1.5);
}

TEST(NumericalMechanismsTest, LaplaceBudgetCorrect) {
  auto distro = absl::make_unique<MockLaplaceDistribution>();
  EXPECT_CALL(*distro, Sample(1.0)).Times(1);
  EXPECT_CALL(*distro, Sample(2.0)).Times(1);
  EXPECT_CALL(*distro, Sample(4.0)).Times(1);
  LaplaceMechanism mechanism(1.0, 1.0, std::move(distro));

  mechanism.AddNoise(0.0, 1.0);
  mechanism.AddNoise(0.0, 0.5);
  mechanism.AddNoise(0.0, 0.25);
}

TEST(NumericalMechanismsTest, LaplaceWorksForIntegers) {
  auto distro = absl::make_unique<MockLaplaceDistribution>();
  ON_CALL(*distro, Sample(_)).WillByDefault(Return(10.0));
  LaplaceMechanism mechanism(1.0, 1.0, std::move(distro));

  EXPECT_EQ(static_cast<int64_t>(mechanism.AddNoise(0)), 10);
}

TEST(NumericalMechanismsTest, LaplaceConfidenceInterval) {
  double epsilon = 0.5;
  double sensitivity = 1.0;
  double level = .95;
  double budget = .5;
  LaplaceMechanism mechanism(epsilon, sensitivity);
  base::StatusOr<ConfidenceInterval> confidence_interval =
      mechanism.NoiseConfidenceInterval(level, budget);
  ASSERT_OK(confidence_interval);
  EXPECT_EQ(confidence_interval->lower_bound(),
            log(1 - level) / epsilon / budget);
  EXPECT_EQ(confidence_interval->upper_bound(),
            -log(1 - level) / epsilon / budget);
  EXPECT_EQ(confidence_interval->confidence_level(), level);

  double result = 19.3;
  base::StatusOr<ConfidenceInterval> confidence_interval_with_result =
      mechanism.NoiseConfidenceInterval(level, budget, result);
  ASSERT_OK(confidence_interval_with_result);
  EXPECT_EQ(confidence_interval_with_result->lower_bound(),
            result + (log(1 - level) / epsilon / budget));
  EXPECT_EQ(confidence_interval_with_result->upper_bound(),
            result - (log(1 - level) / epsilon / budget));
  EXPECT_EQ(confidence_interval_with_result->confidence_level(), level);
}

TEST(NumericalMechanismsTest, LaplaceConfidenceIntervalFailsForBudgetNan) {
  LaplaceMechanism mechanism(1.0, 1.0);
  auto failed_confidence_interval = mechanism.NoiseConfidenceInterval(0.5, NAN);
  EXPECT_THAT(failed_confidence_interval,
              StatusIs(base::StatusCode::kInvalidArgument,
                       HasSubstr("privacy_budget has to be in")));
}

TEST(NumericalMechanismsTest,
     LaplaceConfidenceIntervalFailsForConfidenceLevelNan) {
  LaplaceMechanism mechanism(1.0, 1.0);
  auto failed_confidence_interval = mechanism.NoiseConfidenceInterval(NAN, 1.0);
  EXPECT_THAT(failed_confidence_interval,
              StatusIs(base::StatusCode::kInvalidArgument,
                       HasSubstr("Confidence level has to be in")));
}

TYPED_TEST(NumericalMechanismsTest, LaplaceBuilderClone) {
  LaplaceMechanism::Builder test_builder;
  std::unique_ptr<NumericalMechanismBuilder> clone =
      test_builder.SetL1Sensitivity(3).SetEpsilon(1).Clone();
  auto test_mechanism = clone->Build();
  ASSERT_OK(test_mechanism);

  EXPECT_DOUBLE_EQ((*test_mechanism)->GetEpsilon(), 1);
  EXPECT_DOUBLE_EQ(
      dynamic_cast<LaplaceMechanism*>(test_mechanism->get())->GetSensitivity(),
      3);
}

class NoiseIntervalMultipleParametersTests
    : public ::testing::TestWithParam<struct conf_int_params> {};

struct conf_int_params {
  double epsilon;
  double delta;
  double sensitivity;
  double level;
  double budget;
  double result;
  double true_bound;
};

// True bounds calculated using standard deviations of
// 3.4855, 3.60742, 0.367936, respectively.
struct conf_int_params gauss_params1 = {/*epsilon =*/1.2,
                                        /*delta =*/0.3,
                                        /*sensitivity =*/1.0,
                                        /*level =*/.9,
                                        /*budget =*/.5,
                                        /*result =*/0,
                                        /*true_bound =*/-1.9613};

struct conf_int_params gauss_params2 = {/*epsilon =*/1.0,
                                        /*delta =*/0.5,
                                        /*sensitivity =*/1.0,
                                        /*level =*/.95,
                                        /*budget =*/.5,
                                        /*result =*/1.3,
                                        /*true_bound =*/-1.9054};

struct conf_int_params gauss_params3 = {/*epsilon =*/10.0,
                                        /*delta =*/0.5,
                                        /*sensitivity =*/1.0,
                                        /*level =*/.95,
                                        /*budget =*/.75,
                                        /*result =*/2.7,
                                        /*true_bound =*/-0.5154};

INSTANTIATE_TEST_SUITE_P(TestSuite, NoiseIntervalMultipleParametersTests,
                         testing::Values(gauss_params1, gauss_params2,
                                         gauss_params3));

TEST_P(NoiseIntervalMultipleParametersTests, GaussNoiseConfidenceInterval) {
  // Tests the NoiseConfidenceInterval method for Gaussian noise.
  // Standard deviations are pre-calculated using CalculateStdDev
  // in the Gaussian mechanism class. True bounds are also pre-calculated
  // using a confidence interval calcualtor.

  struct conf_int_params params = GetParam();
  double epsilon = params.epsilon;
  double delta = params.delta;
  double sensitivity = params.sensitivity;
  double budget = params.budget;
  double conf_level = params.level;
  double result = params.result;
  double true_lower_bound = params.result + params.true_bound;
  double true_upper_bound = params.result - params.true_bound;

  GaussianMechanism mechanism(epsilon, delta, sensitivity);
  base::StatusOr<ConfidenceInterval> confidence_interval =
      mechanism.NoiseConfidenceInterval(conf_level, budget, result);

  ASSERT_OK(confidence_interval);
  EXPECT_NEAR(confidence_interval->lower_bound(), true_lower_bound, 0.001);
  EXPECT_NEAR(confidence_interval->upper_bound(), true_upper_bound, 0.001);
  EXPECT_EQ(confidence_interval->confidence_level(), conf_level);
}

TEST(NumericalMechanismsTest, LaplaceEstimatesL1WithL0AndLInf) {
  LaplaceMechanism::Builder builder;
  auto mechanism =
      builder.SetEpsilon(1).SetL0Sensitivity(5).SetLInfSensitivity(3).Build();
  ASSERT_OK(mechanism);
  EXPECT_THAT(
      dynamic_cast<LaplaceMechanism*>(mechanism->get())->GetSensitivity(),
      Ge(3));
}

TEST(NumericalMechanismsTest, AddNoise) {
  auto distro = absl::make_unique<MockLaplaceDistribution>();
  double granularity = distro->GetGranularity();
  ON_CALL(*distro, Sample(_)).WillByDefault(Return(10));
  LaplaceMechanism mechanism(1.0, 1.0, std::move(distro));

  double remainder =
      std::fmod(mechanism.AddNoise(0.1 * granularity, 1.0), granularity);
  EXPECT_EQ(remainder, 0);
  EXPECT_THAT(mechanism.AddNoise(0.1 * granularity, 1.0),
              DoubleNear(10.0, 0.000001));
}

TEST(NumericalMechanismsTest, LambdaTooSmall) {
  LaplaceMechanism::Builder test_builder;
  base::StatusOr<std::unique_ptr<NumericalMechanism>> test_mechanism_or =
      test_builder.SetL1Sensitivity(3)
          .SetEpsilon(1.0 / std::pow(10, 100))
          .Build();
  EXPECT_FALSE(test_mechanism_or.ok());
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsDeltaNotSet) {
  GaussianMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL2Sensitivity(1).SetEpsilon(1).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Delta has to be set.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsDeltaNan) {
  GaussianMechanism::Builder test_builder;
  auto failed_build =
      test_builder.SetL2Sensitivity(1).SetEpsilon(1).SetDelta(NAN).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Delta has to be finite.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsDeltaNegative) {
  GaussianMechanism::Builder test_builder;
  auto failed_build =
      test_builder.SetL2Sensitivity(1).SetEpsilon(1).SetDelta(-1).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Delta has to be in the interval.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsDeltaOne) {
  GaussianMechanism::Builder test_builder;
  auto failed_build =
      test_builder.SetL2Sensitivity(1).SetEpsilon(1).SetDelta(1).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Delta has to be in the interval.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsDeltaZero) {
  GaussianMechanism::Builder test_builder;
  auto failed_build =
      test_builder.SetL2Sensitivity(1).SetEpsilon(1).SetDelta(0).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^Delta has to be in the interval.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsL0SensitivityNan) {
  GaussianMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(NAN)
                          .SetLInfSensitivity(1)
                          .SetEpsilon(1)
                          .SetDelta(0.2)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^L0 sensitivity has to be finite.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsLInfSensitivityInfinity) {
  GaussianMechanism::Builder test_builder;
  auto failed_build = test_builder.SetL0Sensitivity(1)
                          .SetLInfSensitivity(INFINITY)
                          .SetEpsilon(1)
                          .SetDelta(0.2)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^LInf sensitivity has to be finite.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsL2SensitivityNan) {
  GaussianMechanism::Builder test_builder;
  auto failed_build =
      test_builder.SetL2Sensitivity(NAN).SetEpsilon(1).SetDelta(0.2).Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(message, MatchesRegex("^L2 sensitivity has to be finite.*"));
}

TEST(NumericalMechanismsTest, GaussianBuilderFailsCalculatedL2SensitivityZero) {
  GaussianMechanism::Builder test_builder;
  auto failed_build = test_builder.SetEpsilon(1)
                          .SetDelta(0.2)
                          // Use very low L0 and LInf sensitivities so that the
                          // calculation of l2 will result in 0.
                          .SetL0Sensitivity(4.94065645841247e-323)
                          .SetLInfSensitivity(5.24566986113514e-317)
                          .Build();
  EXPECT_THAT(failed_build.status().code(),
              Eq(base::StatusCode::kInvalidArgument));
  // Convert message to std::string so that the matcher works in the open source
  // version.
  std::string message(failed_build.status().message());
  EXPECT_THAT(
      message,
      MatchesRegex(
          "^The calculated L2 sensitivity has to be positive and finite.*"));
}

TEST(NumericalMechanismsTest, GaussianMechanismAddsNoise) {
  GaussianMechanism mechanism(1.0, 0.5, 1.0);

  EXPECT_TRUE(mechanism.AddNoise(1.0) != 1.0);
  EXPECT_TRUE(mechanism.AddNoise(1.1) != 1.1);

  // Test values that should be clamped.
  EXPECT_FALSE(std::isnan(mechanism.AddNoise(1.1, 2.0)));
}

TEST(NumericalMechanismsTest,
     GaussianMechanismAddsNoiseForHighEpsilonAndLowDelta) {
  auto test_mechanism = GaussianMechanism::Builder()
                            .SetL2Sensitivity(6.2324042213746395e-184)
                            .SetDelta(2.7161546250836291e-312)
                            .SetEpsilon(1.257239018692402e+232)
                            .Build();
  EXPECT_TRUE(test_mechanism.ok());

  const double raw_value = 2.7161546250836291e-312;
  double noised_value = (*test_mechanism)->AddNoise(raw_value);
  EXPECT_TRUE(std::isfinite(noised_value));
}

TEST(NumericalMechanismsTest, GaussianMechanismNoisedValueAboveThreshold) {
  GaussianMechanism::Builder builder;
  std::unique_ptr<NumericalMechanism> mechanism = builder.SetL2Sensitivity(1)
                                                      .SetEpsilon(1)
                                                      .SetDelta(0.5)
                                                      .Build()
                                                      .ValueOrDie();

  struct TestScenario {
    double input;
    double threshold;
    double expected_probability;
  };

  // To reduce flakiness from randomness, perform multiple trials and declare
  // the test successful if a sufficient expected number of trials provide the
  // expected result.
  std::vector<TestScenario> test_scenarios = {
      {-0.5, -0.5, 0.5000}, {0.0, -0.5, 0.6915}, {0.5, -0.5, 0.8410},
      {-0.5,  0.0, 0.3085}, {0.0,  0.0, 0.5000}, {0.5,  0.0, 0.6915},
      {-0.5,  0.5, 0.1585}, {0.0,  0.5, 0.3085}, {0.5,  0.5, 0.5000},
  };

  double num_above_thresold;
  for (TestScenario ts : test_scenarios) {
    num_above_thresold = 0;
    for (int i = 0; i < kSmallNumSamples; ++i) {
      if (mechanism->NoisedValueAboveThreshold(ts.input, ts.threshold))
        ++num_above_thresold;
    }
    EXPECT_NEAR(num_above_thresold / kSmallNumSamples, ts.expected_probability,
                0.0025);
  }
}

TEST(NumericalMechanismsTest, GaussianBuilderClone) {
  GaussianMechanism::Builder test_builder;
  auto clone =
      test_builder.SetL2Sensitivity(1.2).SetEpsilon(1.1).SetDelta(0.5).Clone();
  auto mechanism = clone->Build();
  ASSERT_OK(mechanism);

  EXPECT_DOUBLE_EQ((*mechanism)->GetEpsilon(), 1.1);
  EXPECT_DOUBLE_EQ(
      dynamic_cast<GaussianMechanism*>(mechanism->get())->GetDelta(), 0.5);
  EXPECT_DOUBLE_EQ(
      dynamic_cast<GaussianMechanism*>(mechanism->get())->GetL2Sensitivity(),
      1.2);
}

TEST(NumericalMechanismsTest, Stddev) {
  GaussianMechanism mechanism(log(3), 0.00001, 1.0);

  EXPECT_DOUBLE_EQ(mechanism.CalculateStddev(log(3), 0.00001), 3.42578125);
}

}  // namespace
}  // namespace differential_privacy
