
#include <pwm.h>
#include <gtest/gtest.h>


TEST(test_pass, invalid_pass_length5) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("Aa9zx"));
}

TEST(test_pass, valid_pass_length6) {
  ASSERT_EQ(PWM_OK, pwm_is_valid_password("Aa9za1"));
}

TEST(test_pass, invvalid_pass_with_two_punct) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("Aa9za1!!"));
}

TEST(test_pass, blacklisted_pass1) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("A123zad!"));
}

TEST(test_pass, min_length_ok) {
  ASSERT_EQ(PWM_OK, pwm_is_valid_password("Aa0aaa"));
}

TEST(test_pass, max_length_ok_with_one_punct) {
  ASSERT_EQ(PWM_OK, pwm_is_valid_password("Aa0aaaaaaa!"));
}

TEST(test_pass, two_punct_invalid) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("Aa0aaa!!"));
}

TEST(test_pass, missing_upper_invalid) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("aa0aaa"));
}

TEST(test_pass, missing_lower_invalid) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("AA0AAA"));
}

TEST(test_pass, missing_digit_invalid) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("Aa!aaaa"));
}

TEST(test_pass, invalid_char_space) {
  ASSERT_EQ(PWM_INVALID_PASSWORD, pwm_is_valid_password("Aa0a a"));
}
// etc ...

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
