# Getting Started

Start using Janus Client SDK is as easy as cloning a git repo on your local machine. In fact, you only need to clone the main repo in your workspace and run the initialization script.

```bash
git clone https://github.com/helloiampau/janus-client
cd janus-client
make
```

The `make` command downloads all the native dependencies you need to build the library on each platform.
Once the `make` command finished, you are ready to link the platform-specific code to your iOS or Android project.

## Android

In this section, we will link the janus-client android library to your application android project.
If you don't know how to create a new Android project, we suggest you read the official android developer [guide](https://developer.android.com/training/basics/firstapp) from google.

> <i class="fas fa-bomb"></i> janus-client is a C++ SDK. So you need to install the [NDK](https://developer.android.com/ndk/guides) on your development machine

First you need to update the `settings.gradle` file, located in your project root folder, by adding a reference to the janus-client library. The file should look similar to the following:

```gradle
include ':app'

include ':janus'
project(':janus').projectDir = new File(settingsDir, '$JANUS_CLIENT_CLONE_DIRECTORY/platforms/android/janus')
```

> <i class="fas fa-bomb"></i> replace `$JANUS_CLIENT_CLONE_DIRECTORY` with the path of the janus-client folder

Now you can reference the janus external library as a dependency of your project by adding the `:janus` name into the dependencies section of the application `build.gradle`.

```gradle
android {
  defaultConfig {
    ...
    minSdkVersion 16
  }
  ...
  compileOptions {
    compileOptions {
      sourceCompatibility JavaVersion.VERSION_1_8
      targetCompatibility JavaVersion.VERSION_1_8
    }
    sourceCompatibility JavaVersion.VERSION_1_8
    targetCompatibility JavaVersion.VERSION_1_8
  }
}

dependencies {
  implementation project(path: ':janus')
  ...
}
```

> <i class="fas fa-bomb"></i> You also need to set the __minSdkVersion__ and the __compatibility__ flags as above otherwise you will face some build errors

As final step you need to declare the required permission in your application `AndroidManifest.xml`

```gradle
<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="com.github.helloiampau.petsapp">

  <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE">
  <uses-permission android:name="android.permission.INTERNET">
  <uses-permission android:name="android.permission.CAMERA">
  <uses-permission android:name="android.permission.RECORD_AUDIO">
  <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE">
  <uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS">

  ...

</manifest>
```

If your configuration goes fine, your project structure should look like the following:

<div style="text-align: center">
  <img src="_media/android-tree.png" alt="Android project tree" width="50%">
</div>

## iOS

TBD

# Create your real-time application

TBD

# Messing around with the library

## The environment

We provide you with a complete playground, based on docker and docker-compose frameworks, you can use during your real-time application development.
If you don't know what docker is, you should be ashamed, and you should read the official [documentation](https://docs.docker.com/)

### Janus

To test your application in a real scenario, you need to run at least one instance of Janus in your environment. We provide a ready-to-use image of the gateway you can run with:

```bash
docker-compose run -p 8188:8188 -p 8088:8088 -p 5001:5000 -p 9000-9020:9000-9020/udp janus
```

> <i class="fas fa-bomb"></i> The 8088 and 8188 are the default Janus API ports, the UDP ports from 9000 to 9020, instead, are used by the RTP streams. Lastly, we use the port 5001 to bind the [gdbgui](https://www.gdbgui.com) instance we use to debug Janus

### C++ library test environment

We only trust tests. That's why we build a controlled sandbox to test the main janus-client C++ library. You can run the image using:

```bash
docker-compose run -p 5000:5000 test
```

> <i class="fas fa-bomb"></i> We use the port 5000 to bind the [gdbgui](https://www.gdbgui.com) instance we use to debug the library

### Documentation

You can run a self-hosted version of this documentation by running:

```bash
docker-compose run -p 8080:80 docs
```

## Anatomy of the library

The janus-client SDK is a C++ library

## Debug the C++ layer

Debug C++ code is always quite challenging. That's why we tried to make the things easy by adding to the test docker image a friendly gdb UI called [gdbgui](https://www.gdbgui.com/)

* First, you need to run the test docker image as seen [above](#c-library-test-environment)
* start the gdbgui inside the container with
```bash
docker exec -t -i -w /app $CONTAINER_NAME make debugger
```
* open your browser and open the URL [http://localhost:5000](http://localhost:5000)

and enjoy the spectacular gdbgui debugger interface.

> <i class="fas fa-bomb"></i> To find the `$CONTAINER_NAME` you can use the `docker ps ` command

