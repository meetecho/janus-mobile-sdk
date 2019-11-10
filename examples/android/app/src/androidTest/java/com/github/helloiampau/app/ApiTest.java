package com.github.helloiampau.app;

import android.widget.TextView;

import com.azimolabs.conditionwatcher.ConditionWatcher;
import com.azimolabs.conditionwatcher.Instruction;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.matcher.ViewMatchers.withId;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class ApiTest {

  @Rule
  public ActivityTestRule<Api> activityRule = new ActivityTestRule<>(Api.class);

  @Test
  public void shouldConnectToJanus() throws Exception {
    onView(withId(R.id.api_test_start_button)).perform(click());

    TextView statusText = this.activityRule.getActivity().findViewById(R.id.api_test_status_text);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for READY status";
      }

      @Override
      public boolean checkCondition() {
        return statusText.getText().toString().equals("READY");
      }
    });

    onView(withId(R.id.api_test_stop_button)).perform(click());
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for CLOSED status";
      }

      @Override
      public boolean checkCondition() {
        return statusText.getText().toString().equals("CLOSED");
      }
    });
  }

}
