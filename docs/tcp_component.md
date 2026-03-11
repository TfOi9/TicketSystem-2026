# TCP 组件说明

## 概览

TCP 组件位于 `frontend/include/web` 与 `frontend/src/web`，由三层组成：

- TLV 层：`tlvpacket.hpp/cpp`、`tlvparser.hpp/cpp`
- 分发层：`dispatcher.hpp/cpp`、`sender.hpp`
- TCP 层：`tcpclient.hpp/cpp`、`tcpserver.hpp/cpp`

整体流程如下：

1. 发送侧把对象序列化为 payload。
2. 使用 TLV 格式封包（type + length + payload）。
3. 对端解析 TLV 后，按 type 分发给注册的处理器。

## 关键设计

### Dispatcher（接收）

Dispatcher 以 `type` 作为键：

- `registerHandlerWithPayload(type, antiSerializer, handler)` 注册反序列化与处理器。
- `dispatch(type, payload)` 按 type 查找并执行。

### Sender（发送）

Sender 也以 `type` 作为键，与 Dispatcher 保持一致：

- `registerSender(type, serializer)`：序列化函数返回 `QByteArray`。
- `registerSenderWithPayload(type, serializer)`：序列化函数返回 `bool` 并写入 `QByteArray&`。
- `send(type, obj, outPayload)`：按 type 选择序列化器。

这意味着注册与调用都走同一条主线：按 type 注册、按 type 调用。

## TCPClient 使用方法

### 1. 创建并连接

```cpp
sjtu::TCPClient client;
client.connectToServer("127.0.0.1", 34789);
```

### 2. 注册收发编解码

```cpp
constexpr uint32_t kTypeUser = 1001;

struct UserInfo {
    qint32 id;
    QString name;
};

auto serializeUser = [](const UserInfo& user) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << user.id << user.name;
    return payload;
};

auto parseUser = [](const QByteArray& payload, UserInfo& outUser) {
    QDataStream in(payload);
    in >> outUser.id >> outUser.name;
    return in.status() == QDataStream::Ok;
};

client.registerPacketCodec<UserInfo>(
    kTypeUser,
    serializeUser,
    parseUser,
    [](const UserInfo& user) {
        // 收到 UserInfo
    }
);
```

### 3. 发送对象

```cpp
UserInfo u{1, "Alice"};
bool ok = client.sendObject(kTypeUser, u);
```

文本消息兼容接口仍可直接使用：

```cpp
client.send("hello");
```

## TCPServer 使用方法

### 1. 启动服务

```cpp
sjtu::TCPServer server;
server.start(34789);
```

### 2. 注册处理器

```cpp
server.registerPacketReceiver<UserInfo>(
    kTypeUser,
    parseUser,
    [](QTcpSocket* sock, const UserInfo& user) {
        // 处理客户端发来的 user
    }
);
```

### 3. 发送对象给单个客户端或广播

```cpp
UserInfo push{2, "Bob"};
server.sendObject(kTypeUser, push, socket);
server.broadcastObject(kTypeUser, push);
```

文本消息同样保留：

```cpp
server.send("welcome", socket);
server.broadcast("announcement");
```

## 测试与验证

本仓库已将 TCP/TLV 与基础单测接入 CTest，且加入大测试入口：

- 单测：`fixed_string_test`、`type_helper_test`
- TLV/TCP：`tlvpacket_test`、`tlvparser_test`、`dispatcher_test`、`dispatcher_large_test`、`tcp_test`、`tcp_tlv_mixed_test`、`tcp_robustness_test`、`tcp_large_robustness_test`
- 大测试：`ticket_full_regression`（调用 `tools/test.sh`）

执行方式：

```bash
ctest --output-on-failure
```

如需仅运行 TCP/TLV：

```bash
ctest -R "tcp|tlv|dispatcher" --output-on-failure
```
