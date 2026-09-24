QT       += core gui sql serialport serialbus printsupport

# Qt6 必须显式添加 widgets 模块
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 升级 C++ 标准至 C++17（Qt6 + 新版QCustomPlot 推荐）
CONFIG += c++17
# 解决中文编码
CONFIG += utf8_source

# ========== 关键修复：MinGW 大目标文件参数（专治 qcustomplot.o 报错） ==========
QMAKE_CXXFLAGS += -Wa,-mbig-obj

# 编译输出目录（可选，保持整洁）
DESTDIR = bin
OBJECTS_DIR = bin/tmp
MOC_DIR = bin/tmp
RCC_DIR = bin/tmp
UI_DIR = bin/tmp

# 所有源码文件（核对文件名，确保无遗漏）
SOURCES += \
    alarmmanager.cpp \
    curvedialog.cpp \
    databasehelper.cpp \
    historydialog.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    modbusmanager.cpp \
    thresholddialog.cpp \
    usermanagerdialog.cpp \
    qcustomplot.cpp

# 所有头文件
HEADERS += \
    alarmmanager.h \
    curvedialog.h \
    databasehelper.h \
    historydialog.h \
    loginwidget.h \
    mainwindow.h \
    modbusmanager.h \
    thresholddialog.h \
    usermanagerdialog.h \
    qcustomplot.h

# 部署规则（默认无需修改）
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc