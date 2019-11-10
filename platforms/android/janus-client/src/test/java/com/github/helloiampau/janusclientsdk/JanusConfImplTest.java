package com.github.helloiampau.janusclientsdk;

import org.junit.Test;

import static org.junit.Assert.*;

public class JanusConfImplTest {

  @Test
  public void itShouldHandleTheUrlField() {
    JanusConfImpl conf = new JanusConfImpl();
    conf.url("yolo");

    assertEquals(conf.url(), "yolo");
  }

  @Test
  public void itShouldHandleThePluginField() {
    JanusConfImpl conf = new JanusConfImpl();
    conf.plugin("my yolo plugin");

    assertEquals(conf.plugin(), "my yolo plugin");
  }

}