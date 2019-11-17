# Messing around with the library

Suppose you already integrated Janus in your own server-side environment by either installing it behind a proxy API or by building your custom plugin.
Janus-Client SDK meets your needs by providing you with a set of handly interfaces you can use to inject your custom business logic within the framework. And, if you don't like C++ as the default programming language, you can use Java for Android and Objective C for iOS as well!

Take a look at the `JanusFactory` class:

<!-- tabs:start -->

#### **C++**

```cpp
TBD
```

#### **Java**

```java
public class JanusFactory {
  public void protocol(Protocol protocol) {
    ...
  }

  public void pluginFactory(String id, PluginFactory factory) {
    ...
  }

  ...
}
```

#### **Objective C**

```objectivec
TBD
```

<!-- tabs:end -->

it has two magical methods `protocol(Protocol protocol)` and `pluginFactory(String id, PluginFactory factory)` you can use to register your custom protocol and your custom plugins.

So, once you have your custom classes created, you only need to pass them to one of these two methods **BEFORE** creating the Janus service like this:

<!-- tabs:start -->

#### **C++**

```cpp
TBD
```

#### **Java**

```java
JanusFactory factory = new JanusFactory();

// replace the Janus API with your custom protocol
CustomProtocol protocol = new CustomProtocol();
factory.protocol(protocol);

// adding a useless plugin
CustomPluginFactory pluginFactory = new CustomPluginFactory();
factory.pluginFactory("my.yolo.plugin", pluginFactory);

Janus janus = factory.create(conf, self, activity.getApplicationContext());
```

#### **Objective C**

```objectivec
TBD
```

<!-- tabs:end -->

and we are OK. Easy, isn't it?

## Implement your custom protocol

If your Janus deploy does not involve using the Janus API, you can still use the `janus-mobile` framework by extending it with your custom business logic.
All you need is to create your custom protocol class by extending the `Protocol` abstract class. You can see its interface depicted below:

<!-- tabs:start -->

#### **C++**

```cpp
class Protocol {
  public:
    virtual std::string name() = 0;
    virtual void init(const std::shared_ptr<JanusConf> & conf, const std::shared_ptr<Platform> & platform, const std::shared_ptr<ProtocolDelegate> & delegate) = 0;
    virtual void dispatch(const std::string & command, const std::shared_ptr<Bundle> & payload) = 0;
    virtual void hangup() = 0;
    virtual void close() = 0;
    virtual void onOffer(const std::string & sdp, const std::shared_ptr<Bundle> & context) = 0;
    virtual void onAnswer(const std::string & sdp, const std::shared_ptr<Bundle> & context) = 0;
    virtual void onIceCandidate(const std::string & mid, int32_t index, const std::string & sdp, int64_t id) = 0;
    virtual void onIceCompleted(int64_t id) = 0;
};
```

#### **Java**

```java
public abstract class Protocol {
    public abstract String name();
    public abstract void init(JanusConf conf, Platform platform, ProtocolDelegate delegate);
    public abstract void dispatch(String command, Bundle payload);
    public abstract void hangup();
    public abstract void close();
    public abstract void onOffer(String sdp, Bundle context);
    public abstract void onAnswer(String sdp, Bundle context);
    public abstract void onIceCandidate(String mid, int index, String sdp, long id);
    public abstract void onIceCompleted(long id);
}
```

#### **Objective C**

```objectivec
TBD
```

<!-- tabs:end -->

##### `name()`
The `name()` method should return a constant string value you can use  to identify your protocol

##### `init(JanusConf, Platform, ProtocolDelegate)`
The user calls the `init()` method during the initialization phase of the framework. Here you receive:
* An instance of the `JanusConf` object containing the current SDK configuration (Janus endpoint, plugin, and so on). You can customize the JanusConf object by extending its interface;
* An instance of the `Platform` object. You should use it to create both plugin (if you are still using this pattern in your custom plugin) and `Peer` instances (implementing the WebRTC interface);
* The `ProtocolDelegate` object you should use to notify the main program about `onReady()`, `onClose()`,  `onError()`, `onEvent()` and `onHangout()` events.

##### `dispatch(Command, Bundle)`
The `dispatch()` method is used by the user to send commands to your protocol. The `Command` argument is a plain string you can use to identify the requested command, and the `Bundle` object defines the arguments for that command.

##### `hangup()`
This method should implement your protocol hangup procedure

##### `close()`
This method should implement your protocol close procedure

##### `Events`
A `Peer` object uses the `Protocol` object as a delegate by sending all the WebRTC events to it:
* `onOffer`
* `onAnswer`
* `onIceCandidate`
* `onIceCompleted` 

## Extend with your custom plugin

TBD

## The environment

We provide you with a complete playground, based on docker and docker-compose frameworks, you can use during your real-time application development.
If you don't know what docker is, you should be ashamed, and you should read the official [documentation](https://docs.docker.com/)

If you have a working installation of docker and docker-compose on your machine, you can start the entire development environment by running

```bash
docker-compose up
```

from the root folder of the project.

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
