# 火车票管理系统

## 简介
本仓库为一个简化版 12306 火车票管理系统的实现，主体采用 C++ 语言设计，文件存储采取外存 B+ 树结构。当前实现了基于 CLI 界面的交互、B+ 树缓存结构和空间回收，同时在前端部分内实现了服务器 - 客户端分离。未来计划实现客户端 GUI 交互界面和支持并发的 B+ 树，并使主体支持高并发操作。

## 代码文件结构
本仓库的代码结构图如下所示：
```
.
├── CMakeLists.txt
├── README.md
├── docs
│   └── management_system.md
├── include
│   ├── command
│   │   ├── command.hpp
│   │   └── token.hpp
│   ├── config.hpp
│   ├── stl
│   │   ├── exceptions.hpp
│   │   ├── hash_table.hpp
│   │   ├── list.hpp
│   │   ├── priority_queue.hpp
│   │   ├── unordered_map.hpp
│   │   ├── unordered_set.hpp
│   │   └── vector.hpp
│   ├── storage
│   │   ├── bpt.hpp
│   │   ├── buffer.hpp
│   │   ├── disk.hpp
│   │   ├── dynamic_river.hpp
│   │   ├── memory_river.hpp
│   │   └── page.hpp
│   ├── system
│   │   ├── order.hpp
│   │   ├── ticket.hpp
│   │   ├── train.hpp
│   │   └── user.hpp
│   └── utils
│       ├── comparator.hpp
│       ├── fixed_string.hpp
│       ├── time_date.hpp
│       ├── type_helper.hpp
│       └── validator.hpp
└── src
    ├── bpt.cpp
    ├── cleanup.cpp
    ├── command
    │   ├── command.cpp
    │   └── token.cpp
    ├── main.cpp
    ├── system
    │   ├── order.cpp
    │   ├── ticket.cpp
    │   ├── train.cpp
    │   └── user.cpp
    └── utils
        ├── time_date.cpp
        └── validator.cpp
```
其中，`docs` 下存储说明文档，`include` 和 `src` 下分别存储代码头文件和实现文件。代码主要分为 `command` - 指令解析器，`stl` - 常用数据结构库，`storage` - 文件存储系统，`system` - 主体系统和 `utils` - 工具库五个部分。

## 模块设计
### 指令解析器
指令解析器包含 `Token` 词元结构体，`TokenStream` 词元流类和 `Command` 指令类。其中，词元流类负责解析一行输入并将其拆解为词元，指令类则可以由一个词元流构造，将词元解析为时间戳、指令名和各个参数。

`Command` 类含有 `arg` 接口，用于访问某一字母对应的参数，以及 `check` 函数，用于检测指令类是否包含所有必须参数以及剩余参数是否是可选参数。

### 常用数据结构库
数据结构库包含名字空间 `sjtu` 下实现的类 STL 容器 `vector`，`list`，`priority_queue`，`unordered_set` 和 `unordered_map`，其中最后两者由哈希表类 `hash_table` 实现，它们的接口与标准 STL 相似。

### 文件存储系统
文件存储系统主要包含顺序文件存储器 `MemoryRiver` 和 `DynamicRiver`，以及 B+ 树类模板 `BPlusTree`。
#### `MemoryRiver`
简单的顺序文件存储结构，要求存储的对象为定长。含有模板参数 `T` - 存储类型和 `info_len` - 文件头部额外的信息存储空间长度。
#### `DynamicRiver`
顺序文件存储结构，支持存储非定长结构，但需提供非定长结构的序列化和反序列化工具。含有模板参数 `T` - 存储类型，`Stringifier` - 序列化器，`AntiStringifier` - 反序列化器和 `SizeCalculator` - 根据序列化的前四个字节计算当前对象大小的工具类。

`DynamicRiver` 相较于 `MemoryRiver` 可以节省非常多的空间（存储火车类时可以节省至少一倍），但是写入和读取有额外的时间代价。此外，这两者暴露的接口完全一致，简单修改代码就可以换用。

实际测试显示，`DynamicRiver` 比 `MemoryRiver` 慢 6%，但使用的硬盘空间仅为 `MemoryRiver` 的 45%，44% 和 22%（对应三个压力测试点），最大可节省高达 300 MiB 外存空间（第三个压力测试点）。

#### `BPlusTree`
B+ 树模板类，含有模板参数 `KeyType` - 键类型和 `ValueType` - 值类型

其中，顺序文件读写类实现在 `disk.hpp` 中，包含原理与 `MemoryRiver` 相似的硬盘读写器 `DiskManager`。B+ 树的页实现在文件 `page.hpp` 中。

`buffer.hpp` 中实现了缓存管理器 `BufferManager`，通过 `get_page` 接口获取只读页，`get_page_mutable` 获取可写类，并用 `mark_dirty` 标记脏页。注意，用完取得的缓存页后需要调用 `finish_use` 来释放。可以调用 `flush` 来清空所有缓存并写回脏页。缓存的大小在 `config.hpp` 中可以调整。

`bpt.hpp` 中包含了 B+ 树的实现。需要注意的是，B+ 树将会自动检测 `KeyType` 和 `ValueType` 是否含有比较运算符，如不含有将会使用默认比较类 `Comparator`，比较内存哈希值。不建议使用默认比较类，因为存在发生哈希冲突的可能（调试压力测试点时观测到了哈希冲突）。

### 主体系统
主体系统包含用户系统 `UserSystem`，火车系统 `TrainSystem`，订单系统 `OrderSystem` 和火车票管理系统 `TicketSystem`。这些系统的接口与标准要求几乎一致，在此不再赘述，以下仅说明各系统的外存存储结构。
#### `UserSystem`
使用 B+ 树保存用户名到用户数据的映射关系。
#### `TrainSystem`
采取索引 - 数据分离存储的方式，使用 `DynamicRiver` 存储火车信息，`MemoryRiver` 存储站点信息，B+ 树存储车次名、站点名与索引之间的映射关系，以及站点索引和火车索引、火车位置之间的映射关系。
#### `OrderSystem`
使用 B+ 树保存用户名到订单的映射关系，以及订单号到候补订单的映射关系。
#### `TicketSystem`
包含其他三个系统，以及一个文件用于存储时间戳，作为订单号。
#### 主程序
主程序直接使用 `TicketSystem`。在主程序收到 SIGINT 或 SIGTERM 信号时，会先捕获信号并写回所有缓存数据，随后再退出程序。

### 工具库
包含多个工具类与函数。
#### `comparator.hpp`
内存哈希比较器，用于 B+ 树默认比较。
#### `fixed_string.hpp`
定长字符串模板类，可以写入外存。
#### `time_date.hpp`
日期与时间工具类。
#### `type_helper.hpp`
比较函数重载检测器。
#### `validator.hpp`
内含字符串检测器与 Unicode 字符解析器，以及多个格式判断和解析函数。

## 前端
前端使用 Qt 搭建，客户端与服务器分离，客户端基于 GUI，服务器基于 CLI。

### 网络模块
本系统的网络结构主要采取局域网内的 TCP 协议通信，同时服务器使用 UDP 协议广播服务器地址，以便局域网内的客户端自动寻找服务器并连接。

为了防止粘包，本网络模块采用 TLV 包传输数据，并实现了类型安全的自动发送和分发系统，基于注册表实现。

### 客户端
TBD

### 服务器
TBD

## 未来计划
- 进一步优化时间性能，例如实现火车元数据和车票数据分离存储、使用更高效的换乘查询算法等；
- 加入更多接口，例如实现火车多程中转查询功能、支持多个换乘查询结果等；
- 在已有基础上，实现客户端的图形用户界面；
- 实现支持并发的 B+ 树，并使主体支持高并发操作。

##
作者：2025 级 ACM 班 赵睿城

文档更新时间：2026年3月13日