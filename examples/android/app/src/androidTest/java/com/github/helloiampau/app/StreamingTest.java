package com.github.helloiampau.app;

import android.graphics.Bitmap;
import android.widget.ListView;
import android.widget.TextView;

import com.azimolabs.conditionwatcher.ConditionWatcher;
import com.azimolabs.conditionwatcher.Instruction;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.webrtc.EglRenderer;
import org.webrtc.SurfaceViewRenderer;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static org.hamcrest.CoreMatchers.anything;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class StreamingTest {

  @Rule
  public ActivityTestRule<Streaming> activityRule = new ActivityTestRule<>(Streaming.class);

  @Test
  public void shouldConnectToJanus() throws Exception {
    onView(withId(R.id.streaming_test_start_button)).perform(click());

    TextView statusText = this.activityRule.getActivity().findViewById(R.id.streaming_test_status_text);
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

    ListView list = this.activityRule.getActivity().findViewById(R.id.streaming_test_feed_list);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for items";
      }

      @Override
      public boolean checkCondition() {
        return list.getAdapter().getCount() > 0;
      }
    });

    onData(anything()).inAdapterView(withId(R.id.streaming_test_feed_list)).atPosition(0).perform(click());

    TextView streamingStatusView = this.activityRule.getActivity().findViewById(R.id.streaming_test_status);
    ConditionWatcher.waitForCondition(new Instruction() {
      @Override
      public String getDescription() {
        return "Waiting for streaming status";
      }

      @Override
      public boolean checkCondition() {
        return streamingStatusView.getText().toString().equals("streaming: true");
      }
    });

    onView(withId(R.id.streaming_test_stop_button)).perform(click());
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
