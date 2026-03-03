# 火车票管理系统

## 简介
本仓库为一个简化版 12306 火车票管理系统的实现，主体采用 C++ 语言设计，文件存储采取外存 B+ 树结构。当前实现了基于 CLI 界面的交互、B+ 树缓存结构和空间回收。未来计划实现 GUI 交互界面和支持并发的 B+ 树，并使主体支持高并发操作。

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

#### `BPlusTree`
B+ 树模板类