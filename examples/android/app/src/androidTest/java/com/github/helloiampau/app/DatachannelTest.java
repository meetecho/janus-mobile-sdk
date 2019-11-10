package com.github.helloiampau.app;

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

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.action.ViewActions.typeText;
import static androidx.test.espresso.matcher.ViewMatchers.withId;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class DatachannelTest {

  @Rule
  public ActivityTestRule<Datachannel> activityRule = new ActivityTestRule<>(Datachannel.class);

  @Test
  public void shouldSendDataViaDatachannel() throws Exception {
    onView(withId(R.id.datachannel_test_start_button)).perform(click());

    Button sendButton = this.activityRule.getActivity().findViewById(R.id.datachannel_test_send_button);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for send button";
      }

      @Override
      public boolean checkCondition() {
        return sendButton.isEnabled();
      }
    });

    onView(withId(R.id.datachannel_test_message_edit)).perform(click()).perform(typeText("Yolo 420 69"));
    onView(withId(R.id.datachannel_test_send_button)).perform(click());

    TextView log = this.activityRule.getActivity().findViewById(R.id.datachannel_test_messages);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for CLOSED status";
      }

      @Override
      public boolean checkCondition() {
        return log.getText().toString().equals("Janus EchoTest here! You wrote: Yolo 420 69\n");
      }
    });
  }

}
