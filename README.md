Stanford CS144 — 项目概要 （中文） | English: `README.en.md`
===========================================================

本仓库包含来自 Stanford CS144 Networking 实验（minnow 实现）的代码与若干练习/检查点。此 README 给出总体说明、如何构建/运行测试，以及项目的主要文件结构与模块简介，方便快速上手阅读与测试。

### 快速开始（构建与运行测试）

- 在项目根目录下执行以下命令来配置并构建（基于 CMake）：

```bash
cmake -S . -B build
cmake --build build
```

- 运行所有测试（CMake 的 `test` 目标）：

```bash
cmake --build build --target test
```

（可选）使用更详细的 ctest 输出：

```bash
ctest --test-dir build -V
```

- 其他常用目标（如仓库内 `minnow/README.md` 所示）：
  - `cmake --build build --target speed`：运行速度基准
  - `cmake --build build --target tidy`：运行 clang-tidy 建议
  - `cmake --build build --target format`：自动格式化代码

### 项目文件结构（概要）

下面列出仓库中常见的顶层目录与 `minnow` 子目录结构（非穷尽，供快速导航）：

- `Check4_Analyzing_Data/`, `Checkpoint0/` — 课程作业或练习点（按 checkpoint 分类）
- `minnow/` — 主要网络栈实现与测试（参考实验代码）
  - `minnow/CMakeLists.txt` — 构建入口（CMake）
  - `minnow/src/` — 源码实现（协议栈各模块）
    - `wrapping_integers.hh` — Wrap32 类型：用于 32-bit 序号的封装与 unwrap/wrap 操作
    - `network_interface.hh` — NetworkInterface：处理以太网帧、ARP 缓存、转发与接收逻辑
    - 其他以 `tcp_*`, `byte_stream*`, `reassembler*`, `router*` 等命名的文件实现了 TCP、字节流、重组器、路由器等模块
  - `minnow/tests/` — 单元/集成测试与测试驱动
  - `minnow/apps/` — 示例应用或演示程序
  - `minnow/scripts/`、`minnow/util/`、`minnow/etc/` — 工具脚本、辅助工具与配置
  - `minnow/writeups/` — 可能包含实验报告或说明文档
  - `minnow/build/` — 构建输出（可在本地生成）

### 主要模块说明（简短）

- `wrapping_integers.*`：处理 32-bit 序号的封装，提供 `wrap`/`unwrap` 语义，用于序列号算术与比较。
- `network_interface.*`：网络接口层，把 InternetDatagram 封装为以太网帧、维护 ARP 缓存与待发队列、处理 ARP 请求/回复、并按需发送帧到物理输出端口。
- `tcp_*`, `byte_stream*`, `reassembler*`：实现简化的 TCP 发送/接收逻辑、字节流接口与段重组。测试目录包含大量针对这些组件的单元测试（请在 `minnow/tests/` 中查看）。

### 运行指定测试 / 调试

- 若想运行匹配某个名字的测试，可以使用 `ctest` 的正则过滤，例如：

```bash
ctest --test-dir build -V -R wrapping_integers
```

- 也可以直接运行 `build/tests/` 下的某个测试可执行文件（路径视 CMake 配置而定）。
### 标准文件结构（导航）

下面给出仓库的标准文件结构树（供快速导航，省略某些自动生成或临时目录）。你可以按需递归到更深层级（例如 `minnow/src/`）：

```text
/
├── Checkpoint0/
├── Check4_Analyzing_Data/
├── minnow/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── build/                 # 本地构建输出（可忽略）
│   ├── src/
│   │   ├── wrapping_integers.hh
│   │   ├── network_interface.hh
│   │   ├── tcp_sender.hh
│   │   ├── byte_stream.hh
│   │   ├── reassembler.hh
│   │   └── ...                # 其他协议栈实现文件（tcp_*, byte_stream*, router*, 等）
│   ├── tests/                 # 单元与集成测试
│   ├── apps/                  # 示例/演示程序
│   ├── scripts/               # 辅助脚本（构建/测试/运行）
│   ├── util/                  # 工具/公用代码
│   └── writeups/              # 实验报告与说明
├── .gitignore
└── README.md
```

注：
- `minnow/src/` 包含核心实现；若需要我可以把 `src/` 展开为完整文件列表并为每个文件写一句话描述（例如对 `wrapping_integers.hh`、`network_interface.hh`、各个 `tcp_*`、`reassembler*` 等文件分别备注）。
- `minnow/tests/` 下的测试通常由 CTest 管理，构建后测试可在 `build/` 中找到对应的可执行文件。


