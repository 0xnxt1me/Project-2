
#include <pwm.h>
#include <gtest/gtest.h>


TEST(test_user, short_length) {
  ASSERT_EQ(PWM_INVALID_USER_ID, pwm_is_valid_user("rui"));
}

TEST(test_user, invalid_char) {
  ASSERT_EQ(PWM_INVALID_USER_ID, pwm_is_valid_user("@rui"));
}

TEST(test_user, valid_user) {
  ASSERT_EQ(PWM_OK, pwm_is_valid_user("ruiruiruir"));
}

TEST(test_user, valid_user2) {
  ASSERT_EQ(PWM_OK, pwm_is_valid_user("ruiruirui"));
}

// etc ...

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
