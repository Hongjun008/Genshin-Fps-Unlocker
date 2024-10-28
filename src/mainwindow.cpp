#include "mainwindow.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QTimer>
#include <QDir>
#include <QValidator>
#include <QStyleOption>
#include <QPainter>
#include <QGraphicsBlurEffect>
#include "ui_mainwindow.h"

MainWindow::MainWindow(const char *titleText)
    : int_Counter(0), pid(0), int_UpperLimit_FPS(120),
      configured_Memory(false), is_ProcessRunning(false), is_atMainPage(true),
      timer_AutoDetect(new QTimer(this)), timer_GetModule(new QTimer(this)), timer_Monitor(new QTimer(this)),
      img_background(new QPixmap(":/img/bgimg.jpg")),
      ui(new MainWindowUI)
{
    ui->setupUi(this);
    this->setTitle(titleText);
    this->LoadAppConfig();
    this->LoadGameConfig();
}

MainWindow::~MainWindow()
{
    delete img_background;
    delete ui;
}

void MainWindow::Msg(const QString &msg, MsgType type)
{
    ui->info_bar_text->setText(msg);
    ui->info_bar->setToolTip(msg);
    ui->info_bar->setVisible(true);
    auto appendstyle = ui->info_bar->styleSheet().append("QWidget{border-color:%1;background-color:%2;}");
    QString color_bg, color_border, logHeader{};
    switch (type)
    {
    case Information:
        color_border = "rgb(26, 118, 198)";
        color_bg = "rgb(150, 225, 255)";
        ui->info_bar_icon->setText("i");
        break;
    case Warning:
        color_border = "rgb(157, 93, 0)";
        color_bg = "rgb(255, 244, 206)";
        ui->info_bar_icon->setText("!");
        logHeader = "[Warning]";
        break;
    case Error:
        color_border = "red";
        color_bg = "rgb(253, 231, 233)";
        ui->info_bar_icon->setText("⨉");
        logHeader = "[Error]";
        break;
    case LogMsg:
        ui->info_bar->setVisible(false);
        return;
    }
    framework::Log(logHeader + msg);
    ui->info_bar->setStyleSheet(appendstyle.arg(color_border).arg(color_bg));
    ui->info_bar_icon->setStyleSheet(ui->info_bar_icon->styleSheet().append(QString("background-color:%1;").arg(color_border)));
    this->resizeEvent(nullptr);
}

void MainWindow::LoadAppConfig()
{
    QStringList CFGList{"nativeWindow", "startAtFirst", "noAnimation"};
    foreach (auto value, CFGList)
        this->cfg.insert(value, framework::ReadConfigBoolean("App", value.toLocal8Bit()));
    connect(this->timer_AutoDetect, &QTimer::timeout, this, &MainWindow::DetectGamePath);
    connect(this->timer_Monitor, &QTimer::timeout, this, &MainWindow::ProcessMonitor);
    connect(this, &MainWindow::MSG, this, &MainWindow::Msg);
    this->InitializeGUI();
    this->RegisterHotKeys();
}

void MainWindow::LoadGameConfig()
{
    this->LoadSettingsToUI();
    if (framework::ReadConfig(Genshin, "PATH", info_Path_Genshin))
    {
        ui->input_PATH->setText(info_Path_Genshin);
        ui->input_PATH->setToolTip(info_Path_Genshin);
        if (!framework::ReadConfig(Genshin, "FPS", this->int_UpperLimit_FPS))
        {
            this->int_UpperLimit_FPS = DEFAULT_FPS;
            framework::WriteConfig(Genshin, "FPS", this->int_UpperLimit_FPS);
        }
        auto szUpperLimitFPS = QString::number(this->int_UpperLimit_FPS);
        ui->input_value->setText(szUpperLimitFPS);
        ui->text_Value->setText("Target:" + szUpperLimitFPS);
        ui->text_FPS_display->setText("Current Value:" + szUpperLimitFPS);
        connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::StartProcess);
        if (cfg["startAtFirst"])
        {
            this->StartProcess();
            cfg["startAtFirst"] = false;
        }
        return;
    }
    this->Msg("<Genshin-PATH> load failed.", Warning);
    ui->pushButton->disconnect();
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::StartDetectProcess);
}

void MainWindow::InitializeGUI()
{
    const QString szLimitRE("^(60|[6-9]\\d|[1-4]\\d{2}|%1)$");
    const QRegularExpressionValidator limitedValidator(QRegularExpression{szLimitRE.arg(UPPER_LIMIT)});
    ui->input_FPS->setValidator(&limitedValidator);
    ui->input_value->setValidator(&limitedValidator);
    ui->input_value->installEventFilter(this);
    ui->input_PATH->installEventFilter(this);
    ui->info_bar->setVisible(false);
    ui->info_bar_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->info_bar_icon->installEventFilter(this);
    ui->btn_nativeWindowValueChangedRestart->setVisible(false);
    ui->setB_na->setChecked(cfg["noAnimation"]);
    ui->setB_sf->setChecked(cfg["startAtFirst"]);

    connect(ui->btn_Close, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->btn_Minimize, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->btn_Apply_fpsvalue, &QPushButton::clicked, this, &MainWindow::ApplyLimit);
    connect(ui->btn_Settings, &QPushButton::clicked, this, &MainWindow::SwitchPage);
    connect(ui->btn_back, &QPushButton::clicked, this, &MainWindow::SwitchPage);
    connect(ui->btn_up, &QPushButton::clicked, this, &MainWindow::setLimitPlus);
    connect(ui->btn_down, &QPushButton::clicked, this, &MainWindow::setLimitMinus);
    connect(ui->btn_Apply_Settings, &QPushButton::clicked, this, &MainWindow::ApplySettings);
    connect(ui->btn_reload, &QPushButton::clicked, this, &MainWindow::LoadSettingsToUI);
    connect(ui->btn_About, &QPushButton::clicked, this, &MainWindow::DisplayPage_About);

    if (cfg["nativeWindow"])
    {
        this->ui->titleBar->hide();
        ui->setB_nw->setChecked(true);
    }
    else
    {
        ui->setB_nw->setChecked(false);
        MARGINS margins = {-1, -1, -1, -1};
        this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
        auto hwnd = reinterpret_cast<HWND>(this->winId());
        auto style = GetWindowLongW(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION;
        ::SetWindowLongW(hwnd, GWL_STYLE, style);
        ::DwmExtendFrameIntoClientArea(hwnd, &margins);
        ui->titleBar->setEnabled(true);
        ui->titleBar->show();
        ui->titleBar->move(0, 0);
        // setAutoFillBackground(true);
        setAttribute(Qt::WA_TranslucentBackground);
    }
    // auto eff = new QGraphicsBlurEffect(this);
    // eff->setBlurRadius(10);
    // ui->Image_Background->setGraphicsEffect(eff);
    ui->Image_Background->setAlignment(Qt::AlignCenter);
    this->resizeEvent(nullptr);
}

void MainWindow::StartProcess()
{
    if(is_ProcessRunning)
        return;
    STARTUPINFOA info_Startup{};
    BOOL result = CreateProcessA(
        info_Path_Genshin.toLocal8Bit(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        NULL,
        nullptr,
        info_Path_Genshin.left(info_Path_Genshin.lastIndexOf("\\")).toLocal8Bit(),
        &info_Startup,
        &info_Process);
    if (result)
    {
        is_ProcessRunning = true;
        ::CloseHandle(info_Process.hThread);
        ::SetPriorityClass(info_Process.hProcess, HIGH_PRIORITY_CLASS);
        this->pid = info_Process.dwProcessId;
        this->int_Counter = 0;
        connect(this->timer_GetModule, &QTimer::timeout, [this]()
                {
                    if (this->int_Counter++ > 100)
                    {
                        this->Msg("Module load failed", Error);
                        this->int_Counter = 0;
                        this->timer_GetModule->stop();
                        this->timer_GetModule->disconnect();
                        ::CloseHandle(this->info_Process.hProcess);
                        return;
                    }
                    if (!framework::GetModule(this->info_Process.hProcess, info_Path_Genshin.split("\\").last(), &this->module_UnityLib, this->pid))
                    {
                        if (!framework::GetModule(this->info_Process.hProcess, "UnityPlayer.dll", &this->module_UnityLib, this->pid))
                        {
                            if (!framework::GetModule(this->info_Process.hProcess, "unityplayer.dll", &this->module_UnityLib, this->pid))
                            {
                                this->Msg(QString("Getting-Module...").append(QString::number(this->int_Counter)));
                                return;
                            }
                        }
                    }
                    this->Msg("Module load success.");
                    this->int_Counter = 0;
                    this->timer_GetModule->stop();
                    this->timer_GetModule->disconnect();
                    this->Memory_Initialize(); });
        this->timer_GetModule->start(50);
    }
}

void MainWindow::DetectGamePath()
{
    if (!int_Counter--)
    {
        this->timer_AutoDetect->stop();
        this->int_Counter = 0;
        this->Msg("Process detecting stopped or timeout", Warning);
        ui->pushButton->setText("START");
        ui->pushButton->disconnect();
        connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::StartDetectProcess);
        return;
    }

    this->pid = framework::GetPID("YuanShen.exe");
    if (!this->pid)
        this->pid = framework::GetPID("GenshinImpact.exe");
    if (!this->pid)
        return;
    int_Counter = 0;
    this->timer_AutoDetect->stop();
    HANDLE h_Process = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE,
        FALSE,
        this->pid);
    if (!h_Process)
        this->Msg(framework::LastError(::GetLastError()), Error);
    else
    {
        char pszPath[LMAXPATH];
        auto lMaxPath = LMAXPATH;
        ::QueryFullProcessImageNameA(h_Process, 0, pszPath, &lMaxPath);
        info_Path_Genshin = pszPath;
        ui->pushButton->disconnect();
        ui->pushButton->setText("START");
        this->Msg("Game path found");
        connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::StartProcess);
        if (!framework::WriteConfig(Genshin, "PATH", info_Path_Genshin))
            emit SIG_Error("Game path found but config write failed");
        this->LoadGameConfig();
    }
}

void MainWindow::ApplyLimit()
{
    this->SetLimit(ui->input_value->text().toInt());
}

void MainWindow::setLimitMinus()
{
    qint32 value = ui->input_value->text().toInt() - 10;
    if (value >= 60 && value <= UPPER_LIMIT)
        ui->input_value->setText(QString::number(value));
}

void MainWindow::setLimitPlus()
{
    qint32 value = ui->input_value->text().toInt() + 10;
    if (value >= 60 && value <= UPPER_LIMIT)
        ui->input_value->setText(QString::number(value));
}

void MainWindow::SetLimit(qint32 value)
{
    if (value < 60)
        value = 60;
    else if (value > UPPER_LIMIT)
        value = UPPER_LIMIT;
    this->int_UpperLimit_FPS = value;
    ui->input_value->setText(QString::number(value));
    ui->text_Value->setText("Current:" + QString::number(value));
}

void MainWindow::ApplySettings()
{
    if (framework::ReadConfigBoolean("App", "nativeWindow") != ui->setB_nw->isChecked())
    {
        ui->btn_nativeWindowValueChangedRestart->show();
        framework::WriteConfigBoolean("App", "nativeWindow", ui->setB_nw->isChecked());
    }
    framework::WriteConfigBoolean("App", "startAtFirst", ui->setB_sf->isChecked());
    framework::WriteConfigBoolean("App", "noAnimation", ui->setB_na->isChecked());

    QString szFPS = ui->input_FPS->text();
    if (!szFPS.isEmpty())
        framework::WriteConfig(Genshin, "FPS", szFPS);
    else
        framework::WriteConfig(Genshin, "FPS", DEFAULT_FPS);
    LoadSettingsToUI();
}

void MainWindow::LoadSettingsToUI()
{
    ui->setB_sf->setChecked(framework::ReadConfigBoolean("App", "startAtFirst"));
    ui->setB_na->setChecked(framework::ReadConfigBoolean("App", "noAnimation"));
    ui->setB_nw->setChecked(framework::ReadConfigBoolean("App", "nativeWindow"));

    QString valuefps;
    if (framework::ReadConfig(Genshin, "FPS", valuefps))
    {
        ui->input_FPS->setText(valuefps);
        ui->text_FPS_display->setText("Current:" + valuefps);
    }
}

void MainWindow::DisplayPage_About()
{
    QMessageBox::about(this, "About 关于", 
    "Genshin Impact FPS Unlocker 原神帧率解锁工具\n"
    "Test version, 0.1\n"
    "By Hongjun008 https://hongjun.tech\n"
    "Thanks/tech from: Github@winTEuser\n"
    "More in https://github.com/Hongjun008/Genshin-Fps-Unlocker\n"
    );
}

void MainWindow::Memory_Initialize()
{
    if (this->module_UnityLib.modBaseAddr == nullptr)
        return emit SIG_Error("UnityLib module address load fail");
    auto pe_buffer = ::VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pe_buffer)
    {
        ::CloseHandle(this->info_Process.hProcess);
        return emit SIG_Error("VirtualAlloc Failed! (PE_buffer)");
    }
    auto UnityLib_mod_baseAddr = reinterpret_cast<quintptr>(this->module_UnityLib.modBaseAddr);
    if (::ReadProcessMemory(info_Process.hProcess, reinterpret_cast<void *>(UnityLib_mod_baseAddr), pe_buffer, 0x1000, nullptr) == 0)
    {
        ::VirtualFree(pe_buffer, 0, MEM_RELEASE);
        ::CloseHandle(info_Process.hProcess);
        return emit SIG_Error("Read Process Memory Failed! (PE_buffer)");
    }
    quint32 textVirtualSize = 0;
    quintptr textRemoteAddr;
    if (framework::GetSectionInfo(pe_buffer, ".text", &textVirtualSize, &textRemoteAddr, UnityLib_mod_baseAddr))
    {
        this->info_Memory.buffer = pe_buffer;
        this->info_Memory.remoteVA = textRemoteAddr;
        this->info_Memory.sectionVS = textVirtualSize;
        this->info_Memory.remoteBA = UnityLib_mod_baseAddr;
        this->Memory_Configure();
        return;
    }
    ::VirtualFree(pe_buffer, 0, MEM_RELEASE);
    ::CloseHandle(info_Process.hProcess);
    return emit SIG_Error("Get Target Section Fail! (text)");
}

void MainWindow::Memory_Configure()
{
    auto copyTextVirtualAddr = ::VirtualAlloc(nullptr, this->info_Memory.sectionVS, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!copyTextVirtualAddr)
    {
        ::CloseHandle(this->info_Process.hProcess);
        return emit SIG_Error("VirtualAlloc Failed! (text)");
    }
    if (!::ReadProcessMemory(this->info_Process.hProcess, reinterpret_cast<void *>(this->info_Memory.remoteVA), copyTextVirtualAddr, this->info_Memory.sectionVS, nullptr))
    {
        ::VirtualFree(copyTextVirtualAddr, 0, MEM_RELEASE);
        ::CloseHandle(info_Process.hProcess);
        return emit SIG_Error("Memory read Fail ! (text)");
    }
    const char signatureAddr1[] = "7F 0E E8 ?? ?? ?? ?? 66 0F 6E C8";
    const char signatureAddr2[] = "7F 0F 8B 05 ?? ?? ?? ?? 66 0F 6E C8";
    const auto startAddress = reinterpret_cast<quintptr>(copyTextVirtualAddr);
    quintptr ptr_fps = NULL; // normal_fps_ptr

    if (quintptr address = framework::PatternScan_Region(startAddress, this->info_Memory.sectionVS, signatureAddr1))
    {
        address += 3;
        address += *reinterpret_cast<qint32 *>(address) + 6;
        address += *reinterpret_cast<qint32 *>(address) + 4;
        ptr_fps = address - reinterpret_cast<quintptr>(copyTextVirtualAddr) + this->info_Memory.remoteVA;
    }
    else if ((address = framework::PatternScan_Region(startAddress, this->info_Memory.sectionVS, signatureAddr2)))
    {
        address += 4;
        address += *reinterpret_cast<quint32 *>(address) + 4;
        ptr_fps = address - reinterpret_cast<quintptr>(copyTextVirtualAddr) + this->info_Memory.remoteVA;
    }
    else
    {
        ::CloseHandle(info_Process.hProcess);
        ::VirtualFree(copyTextVirtualAddr, 0, MEM_RELEASE);
        return emit SIG_Error("Genshin Pattern may outdated!");
    }
    auto sectionVS_il2cpp = this->info_Memory.sectionVS;
    if (framework::GetSectionInfo(this->info_Memory.buffer, "il2cpp", &sectionVS_il2cpp, &this->info_Memory.remoteVA, this->info_Memory.remoteBA))
    {
        auto Copy_Text_VA = ::VirtualAlloc(nullptr, sectionVS_il2cpp, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (Copy_Text_VA == nullptr)
            emit SIG_Error("VirtualAlloc Failed! (il2cpp_GI)");
        if (!::ReadProcessMemory(info_Process.hProcess, reinterpret_cast<void *>(this->info_Memory.remoteVA), Copy_Text_VA, this->info_Memory.sectionVS, nullptr))
            emit SIG_Error("Memory read Failed! (il2cpp_GI)");
        if (!framework::InjectPatch(Copy_Text_VA, sectionVS_il2cpp, this->info_Memory.remoteVA, ptr_fps, info_Process.hProcess, this->int_UpperLimit_FPS))
            emit SIG_Error("Path inject failed!");
        else
        {
            emit MSG("Use HotKey(R-Ctrl+Up/Down) to control the limit value");
            this->configured_Memory = true;
        }
    }

    ::VirtualFree(this->info_Memory.buffer, 0, MEM_RELEASE);
    ::VirtualFree(copyTextVirtualAddr, 0, MEM_RELEASE);
}

void MainWindow::StartDetectProcess()
{
    this->int_Counter = 200;
    this->timer_AutoDetect->start(200);
    this->Msg("Please start game mannualy");
    ui->pushButton->setText("Stop Detecting");
    ui->pushButton->disconnect();
    connect(ui->pushButton, &QPushButton::clicked, [this]()
            { this->int_Counter = 0; });
}

void MainWindow::ProcessMonitor()
{
    DWORD dwExitCode = 0;
    ::GetExitCodeProcess(this->info_Process.hProcess, &dwExitCode);
    if (dwExitCode == STILL_ACTIVE)
    {
        this->is_ProcessRunning = true;
        return;
    }
    this->is_ProcessRunning = false;
}

void MainWindow::RegisterHotKeys()
{
    bool up = ::RegisterHotKey((HWND)winId(), VK_UP, MOD_CONTROL, VK_UP);
    bool down = ::RegisterHotKey((HWND)winId(), VK_DOWN, MOD_CONTROL, VK_DOWN);
    if (!up || !down)
        this->Msg("HotKey register failed", Warning);
}

void MainWindow::SwitchPage()
{
    auto width = ui->widget->width();
    if (ui->setB_na->isChecked())
    {
        if (is_atMainPage)
        {
            ui->page_Main->move(-width, 0);
            ui->page_Settings->move(0, 0);
            ui->page_Main->hide();
            ui->info_bar->hide();
            ui->page_Settings->show();
        }
        else
        {
            ui->page_Main->move(0, 0);
            ui->page_Settings->move(width, 0);
            ui->page_Main->show();
            ui->page_Settings->hide();
            ui->info_bar->show();
        }
    }
    else
    {
        auto pageMainAnim = new QPropertyAnimation(ui->page_Main, "geometry", this);
        auto pageSettingsAnim = new QPropertyAnimation(ui->page_Settings, "geometry", this);
        auto rectMid = ui->widget->rect();
        auto rectRight = QRect{rectMid.width(), 0, rectMid.width(), rectMid.height()};
        auto rectLeft = QRect{-rectMid.width(), 0, rectMid.width(), rectMid.height()};
        if (is_atMainPage)
        {
            connect(pageMainAnim, &QAbstractAnimation::finished, [pageMainAnim, this]()
                    { pageMainAnim->deleteLater(); dynamic_cast<QWidget*>(pageMainAnim->targetObject())->hide(); });
            connect(pageSettingsAnim, &QAbstractAnimation::finished, [pageSettingsAnim, this]()
                    { pageSettingsAnim->deleteLater(); });
            ui->page_Settings->show();
            pageMainAnim->setStartValue(rectMid);
            pageMainAnim->setEndValue(rectLeft);
            pageSettingsAnim->setStartValue(rectRight);
            pageSettingsAnim->setEndValue(rectMid);
            ui->info_bar->hide();
        }
        else
        {
            connect(pageMainAnim, &QAbstractAnimation::finished, [pageMainAnim]()
                    { pageMainAnim->deleteLater(); });
            connect(pageSettingsAnim, &QAbstractAnimation::finished, [pageSettingsAnim]()
                    { pageSettingsAnim->deleteLater(); dynamic_cast<QWidget*>(pageSettingsAnim->targetObject())->hide(); });
            ui->page_Main->show();
            pageMainAnim->setStartValue(rectLeft);
            pageMainAnim->setEndValue(rectMid);
            pageSettingsAnim->setStartValue(rectMid);
            pageSettingsAnim->setEndValue(rectRight);
            ui->info_bar->show();
        }
        pageMainAnim->setDuration(300);
        pageSettingsAnim->setDuration(300);
        pageMainAnim->setEasingCurve(QEasingCurve::InOutQuad);
        pageSettingsAnim->setEasingCurve(QEasingCurve::InOutQuad);
        pageMainAnim->start();
        pageSettingsAnim->start();
    }
    is_atMainPage = !is_atMainPage;
}

void MainWindow::setTitle(QString titleText)
{
    ui->title_text->setText(titleText);
    return this->setWindowTitle(titleText);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize widgetSize = {ui->page_Main->width(), this->height()};
    auto yPos = 0;
    if (ui->titleBar->isEnabled())
    {
        yPos = ui->titleBar->height();
        ui->titleBar->resize(this->width(), ui->titleBar->height());
        widgetSize.setHeight(widgetSize.height() - yPos);
    }
    ui->widget->move((width() - ui->widget->width()) / 2, yPos);
    ui->widget->resize(widgetSize);
    ui->page_Main->resize(widgetSize);
    ui->page_Settings->resize(widgetSize);
    int infoBarHeight = 35;
    if (ui->info_bar_text->text().contains('\n'))
        infoBarHeight = 55;
    ui->info_bar->setGeometry(100, this->height() - 100 + infoBarHeight, this->width() - 200, infoBarHeight);
    ui->Image_Background->resize(this->size());
    ui->Image_Background->setPixmap(img_background->scaled(ui->Image_Background->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    return QMainWindow::paintEvent(event);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType != "windows_generic_MSG")
        return false;

    ::MSG *msg = static_cast<::MSG *>(message);
    switch (msg->message)
    {
    case WM_HOTKEY:
    {
        if (::GetKeyState(VK_RCONTROL) > 0 || int_Counter > 0)
            return false;
        else if (msg->wParam == VK_UP)
            this->setLimitPlus();
        else if (msg->wParam == VK_DOWN)
            this->setLimitMinus();
        this->ApplyLimit();
    }
    break;
    case WM_NCCALCSIZE:
    {
        *result = 0;
        return true;
    }
    case WM_NCHITTEST:
    {
        auto p = this->mapFromGlobal(QCursor::pos());
        bool l = p.x() < 8;
        bool t = p.y() < 8;
        bool r = p.x() > this->width() - 8;
        bool b = p.y() > this->height() - 8;
        *result = (r && b)                             ? HTBOTTOMRIGHT
                  : (l && b)                           ? HTBOTTOMLEFT
                  : (r && t)                           ? HTTOPRIGHT
                  : (l && t)                           ? HTTOPLEFT
                  : b                                  ? HTBOTTOM
                  : r                                  ? HTRIGHT
                  : l                                  ? HTLEFT
                  : t                                  ? HTTOP
                  : ui->title_text->rect().contains(p) ? HTCAPTION
                                                       : NULL;
        if (*result)
            return true;
    }
    break;
    case WM_GETMINMAXINFO:
    {
        if (::IsZoomed(msg->hwnd))
        {
            ui->titleBar->layout()->setContentsMargins(7, 0, 7, 0);
            setContentsMargins(0, 7, 0, 0);
        }
        else
        {
            QMargins margin0{};
            ui->titleBar->layout()->setContentsMargins(margin0);
            this->setContentsMargins(margin0);
        }
        return false;
    }
    default:
        break;
    }
    return false;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    auto eventType = event->type();
    if (!obj)
        return false;
    else if (obj == ui->info_bar_icon && event->type() == QEvent::Type::MouseButtonPress)
    {
        QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton)
        {
            ui->info_bar->hide();
            return true;
        }
    }
    else if (obj == ui->input_value && event->type() == QEvent::Wheel)
    {
        if (dynamic_cast<QWheelEvent *>(event)->angleDelta().y() > 0)
            this->setLimitPlus();
        else
            this->setLimitMinus();
    }
    else if (obj == ui->input_PATH && eventType == QEvent::MouseButtonPress)
    {
        if (!is_ProcessRunning)
        {
            if (QMessageBox::warning(this, "Warning!", "Start detecting game automaticaly now?", QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok)
            {
                this->SwitchPage();
                this->StartDetectProcess();
            }
        }
    }

    return QObject::eventFilter(obj, event);
}
