
# 工业空压机物联网测控系统

> 📌 English version: [README_EN.md](./README_EN.md)

## 📖 项目简介
**工业空压机物联网测控系统**，基于 Qt6 开发，实现空压机设备数据采集、实时监控、超限报警、历史数据存储、趋势曲线可视化、用户权限管理等完整功能。

- 通信协议：**Modbus‑RTU（串口） / Modbus‑TCP**
- 数据库：SQLite 本地数据库
- 绘图组件：QCustomPlot
- 权限模型：管理员 / 普通用户两级权限

## ✨ 主要功能
1. **设备通信采集**
    - 支持串口 RTU、Modbus‑TCP 两种模式
    - 1秒周期读取空压机10路传感器保持寄存器数据
2. **实时监控界面**
    - 流程图背景展示10项传感器实时数值
    - 运行状态指示灯、报警状态提示
3. **报警管理**
    - 可自定义各路传感器上下限报警阈值
    - 参数超限弹窗报警 + 蜂鸣提示，自动持久化报警记录到数据库
4. **趋势曲线**
    - QCustomPlot 绘制实时趋势曲线
    - 可勾选选择显示指定传感器曲线，鼠标悬浮查看采集数值与时间
5. **历史数据管理**
    - 按时间范围查询传感器历史运行数据
    - 支持历史数据导出 CSV 文件，便于 Excel 分析
6. **用户与权限管理**
    - 用户登录验证，区分**管理员(0)** 和**普通用户(1)**
    - 普通用户仅查看；管理员可修改报警阈值、增删改系统用户
7. **报表打印**
    - 支持运行报表打印输出

## 🧱 技术栈
- GUI框架：Qt 6 (Widgets, Qt Modbus, Qt SQL, Qt SerialPort, Qt PrintSupport)
- 数据库：SQLite3
- 绘图库：QCustomPlot
- 通信：Qt Modbus（RTU / TCP）

## 📁 项目目录结构
```

CompressorMonitor/
├── main.cpp                     # 程序入口
├── mainwindow.h/cpp             # 主监控窗口
├── loginwidget.h/cpp             # 登录对话框
├── modbusmanager.h/cpp          # Modbus 通信管理
├── alarmmanager.h/cpp           # 报警逻辑管理器
├── databasehelper.h/cpp         # SQLite 数据库封装
├── thresholddialog.h/cpp        # 报警阈值设置对话框
├── curvedialog.h/cpp            # 实时趋势曲线窗口
├── historydialog.h/cpp          # 历史数据查询导出
├── usermanagerdialog.h/cpp      # 用户管理对话框
├── qcustomplot.h/cpp            # QCustomPlot 绘图组件
├── res/                             # 资源文件：背景图片等
│   ├── login_bg.png
│   └── main_bg.png
├── res.qrc                          # Qt 资源描述文件
├── CompressorMonitor.pro         # Qt 项目工程文件
README.md                        # 中文主文档
README_EN.md                     # 英文文档

```

## ⚙️ 编译与运行
### 环境要求
- Qt 6.11+ 版本（需要安装组件：`qtmodbus`, `qtserialport`, `qtsql-sqlite`）
- 支持 Windows / Linux

### 编译步骤
1. Clone 本项目到本地
```bash
git clone https://github.com/YourName/AirCompressorMonitor.git
cd AirCompressorMonitor
```

2. 使用 QtCreator 打开 `AirCompressorMonitor.pro`
3. 检查 pro 文件模块配置，确保开启 `modbus、serialport、sql、printsupport`
4. 构建项目，编译生成可执行程序
5. 运行程序
   - 默认账号：
     - 管理员：`admin` / `admin123`
     - 普通用户：`user` / `user123`

> 
> 💡提示：程序运行后自动生成 `compressor_data.db` SQLite 数据库文件。

## ⚠️ 注意事项

1. 资源图片：`res.qrc` 需要正确配置背景图片，缺失会造成登录 / 主窗口背景空白。
2. Modbus：串口模式需要本机存在对应 COM 口；TCP 模式请填写正确设备 IP 与 502 端口。
3. 当前版本寄存器原始数值直接转为 double，**未做工程量缩放换算**，可根据实际设备手册修改数据转换逻辑。
4. 密码目前明文存储，正式生产使用建议增加密码哈希加密。

## 📄 License

MIT License
