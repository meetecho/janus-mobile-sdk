package com.github.helloiampau.app;

import android.Manifest;
import android.widget.Button;
import android.widget.TextView;

import com.azimolabs.conditionwatcher.ConditionWatcher;
import com.azimolabs.conditionwatcher.Instruction;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.rule.GrantPermissionRule;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.matcher.ViewMatchers.withId;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class EchotestTest {

  @Rule
  public ActivityTestRule<Echotest> activityRule = new ActivityTestRule<>(Echotest.class);

  @Rule
  public GrantPermissionRule permissionRule = GrantPermissionRule.grant(Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO);

  @Test
  public void shouldRunTheEchotest() throws Exception {
    onView(withId(R.id.echotest_test_start_button)).perform(click());

    TextView statusText = this.activityRule.getActivity().findViewById(R.id.echotest_test_status_text);
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

    onView(withId(R.id.echotest_test_call_button)).perform(click());

    Button hangup = this.activityRule.getActivity().findViewById(R.id.echotest_test_hangup_button);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for Hangup button";
      }

      @Override
      public boolean checkCondition() {
        return hangup.isEnabled();
      }
    });

    onView(withId(R.id.echotest_test_hangup_button)).perform(click());

    Button call = this.activityRule.getActivity().findViewById(R.id.echotest_test_call_button);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for Call button";
      }

      @Override
      public boolean checkCondition() {
        return call.isEnabled();
      }
    });

    onView(withId(R.id.echotest_test_stop_button)).perform(click());
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
