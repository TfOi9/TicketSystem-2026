# 前端架构文档

## 概述

TicketSystem 前端是一个基于 **Qt6 (C++17)** 的客户端-服务器图形化桌面应用，采用分离的 `client_gui`（图形客户端）和 `server`（TCP服务端）可执行程序。通信层使用 TCP + TLV 协议，通过 UDP 广播实现服务端自动发现。

## 目录结构

```
frontend/
├── include/
│   └── client/
│       ├── main_window.hpp              # 主窗口（所有页面和信号集中管理）
│       ├── bars/
│       │   ├── top_bar.hpp              # 顶栏导航条
│       │   └── status_bar.hpp           # 底部连接状态栏
│       ├── dialogs/
│       │   ├── login_dialog.hpp         # 登录对话框
│       │   ├── register_dialog.hpp      # 注册对话框
│       │   ├── profile_dialog.hpp       # 个人信息查看对话框
│       │   ├── buy_ticket_dialog.hpp    # 购票确认对话框
│       │   ├── refund_dialog.hpp        # 退票确认对话框
│       │   └── add_train_dialog.hpp     # 管理员添加列车对话框
│       └── widgets/
│           ├── home_page_widget.hpp     # 首页（横幅 + 查询表单）
│           ├── ticket_query_widget.hpp  # 车票查询表单
│           ├── ticket_list_widget.hpp   # 车票结果列表
│           ├── orders_page_widget.hpp   # 订单管理页
│           └── admin_page_widget.hpp    # 管理员管理页
├── src/
│   ├── client/
│   │   ├── main.cpp                     # GUI 程序入口
│   │   ├── main_window.cpp              # 主窗口核心逻辑（信号连接、网络处理）
│   │   ├── bars/                        # 顶栏和状态栏实现
│   │   ├── dialogs/                     # 各对话框实现
│   │   └── widgets/                     # 各页面组件实现
│   └── web/                             # 网络通信层
│       ├── server.cpp                   # TCP 服务端入口
│       ├── client.cpp                   # TCP 命令行客户端入口
│       ├── tcp/                         # TCP 客户端/服务端
│       ├── udp/                         # UDP 客户端/服务端
│       ├── dispatch/                    # 类型分派/序列化
│       └── tlv/                         # TLV 封包/解析
```

## 前端架构

### 组件树

```
MainWindow (QMainWindow, 1240x800)
├── TopBar (顶栏导航条, h=50px)
│   ├── 首页 | 购票 | 订单 | 管理 导航按钮
│   └── 登录 | 注册 按钮 / 用户下拉菜单
├── QStackedWidget (页面容器)
│   ├── HomePageWidget (首页)
│   │   ├── TicketQueryWidget (查询表单, 左侧)
│   │   └── 宣传横幅 (右侧)
│   ├── TicketListWidget (车票列表)
│   └── OrdersPageWidget (订单管理)
│   └── AdminPageWidget (管理员管理)
├── StatusBar (底部状态栏)
├── LoginDialog (登录对话框)
├── RegisterDialog (注册对话框)
├── ProfileDialog (个人信息对话框)
├── BuyTicketDialog (购票确认对话框)
├── RefundDialog (退票确认对话框)
└── AddTrainDialog (添加列车对话框, 管理员)
```

### 核心通信模式

```
[GUI 客户端]                    [TCP 服务端]
     │                               │
     │ UDP: "DISCOVER" 广播          │
     │ ────────────────────────────> │
     │ <─ "TicketSystem:<IP>" ─────── │
     │                               │
     │ TCP: connect(IP, 1145)        │
     │ ────────────────────────────> │
     │                               │
     │ TLV(type=1001, Command)       │
     │ ────────────────────────────> │ TicketSystem::handle()
     │ <── TLV(type=2, Result) ────── │
     │                               │
     │ processServerResult() → UI    │
```

### 数据流

1. **用户操作** → Qt 信号/槽机制
2. **MainWindow::sendCommandLine()** → 构建 CLI 格式命令字符串，解析为 `Command` 对象
3. **TCPClient::sendObject(1001, cmd)** → 序列化为 QByteArray，TLV 封装，TCP 发送
4. **服务端 TicketSystem::handle()** → 处理命令，返回 `Result*`
5. **服务端 TCPServer** → 序列化 Result，TLV(type=2) 发回
6. **客户端 TCPClient** → 反序列化 Result，调用 `processServerResult()`
7. **processServerResult()** → 根据 `PendingAction` 分发，更新对应 UI 组件

## 页面功能说明

### 首页 (HomePageWidget)

- 蓝色横向横幅卡片，左侧查询表单，右侧标题「畅行 2026 夏季班次」
- `TicketQueryWidget`：出发站/到达站输入框、换向按钮、日期选择器、换乘复选框、查询按钮
- 支持直通查询（`query_ticket`）和换乘查询（`query_transfer`）

### 车票列表页 (TicketListWidget)

- 表格展示查询结果：车次名、出发站、到达站、出发/到达时间、历时、价格、余票
- 支持按列排序（时间、价格、历时使用数值比较）
- 每行「购票」按钮，点击弹出 `BuyTicketDialog`
- 车次名可点击（蓝色链接样式），点击后弹出该列车时刻表对话框（`query_train`）

### 订单管理页 (OrdersPageWidget)

- 表格展示当前用户订单：序号、车次、出发/到达站、时间、价格、张数、状态
- 状态用颜色区分：已购（绿色）、待候（黄色）、已退（红色）、无效（灰色）
- 已购和待候订单可点击「退票」按钮，弹出 `RefundDialog`
- 顶部「刷新」按钮手动刷新，切换到订单页时自动刷新
- 自动调用 `query_order -u <username>`

### 管理员管理页 (AdminPageWidget)

- **列车管理**：
  - 列车编号输入框 + 查询/发布/删除/添加列车 按钮
  - 查询列车显示列车详情（编号、类型、站点列表、票价、余票）
  - 添加列车弹窗：完整的列车参数表单（站点、座位、票价、时间、日期、类型）
  - 删除列车需要二次确认
- **用户管理**：
  - 用户名输入框 + 查询/添加用户 按钮
  - 查询用户显示用户名、姓名、邮箱、权限等级
  - 添加用户弹窗：用户名、密码、姓名、邮箱、权限等级

### 对话框

| 对话框 | 功能 | 后端命令 |
|---|---|---|
| LoginDialog | 用户名+密码登录 | `login -u -p` |
| RegisterDialog | 用户注册（用户名/密码/姓名/邮箱） | `add_user -c root ...` |
| ProfileDialog | 只读个人信息展示 | `query_profile` |
| BuyTicketDialog | 确认购买：显示车次信息、余票、购票张数、候补选项 | `buy_ticket` |
| RefundDialog | 确认退票：显示订单详情和警告 | `refund_ticket` |
| AddTrainDialog | 完整列车参数表单 (`-i -n -m -s -p -x -t -o -d -y`) | `add_train` |

## 视觉设计规范

### 色彩体系

| 用途 | 色值 |
|---|---|
| 主色调（按钮、顶栏、高亮） | `#3d80de` |
| 主色调悬停 | `#2f6fc6` |
| 页面背景 | `#eef3fb` |
| 白色卡片 | `#ffffff` / `rgba(255,255,255,0.95)` |
| 主文字 | `#1e3a5f` |
| 次要文字 | `#334155` |
| 边框 | `#cbd5e1` |
| 成功状态 | `#16a34a` |
| 警告状态 | `#d97706` |
| 危险/退出 | `#dc2626` |
| 表格表头 | `#f0f6ff` |
| 表格交替行 | `#f8fbff` |

### 文字规格

| 元素 | 字体大小 | 字重 |
|---|---|---|
| 页面标题 | 22px | 700 |
| 横幅标题 | 34px | 700 |
| 顶栏标题 | 20px | 700 |
| 顶栏按钮 | 14px | 400 |
| 对话框标题 | 16px | 600 |
| 表单标签 | 13px | 400 |
| 状态栏 | 13px | 400 |

## 构建说明

### 依赖

- **CMake ≥ 3.16**
- **Qt 6** (Core, Widgets, Network)
- **C++17** 兼容编译器

### 构建命令

```bash
cmake -B build -S .
cmake --build build --target client_gui -j$(nproc)
cmake --build build --target server -j$(nproc)
```

### 运行

```bash
# 先启动服务端
./build/server

# 再启动图形客户端
./build/client_gui
```

客户端通过 UDP 广播自动发现局域网内的服务端，无需手动配置 IP。
