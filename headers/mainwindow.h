#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWindow>
#include <Windows.h>
#include <TlHelp32.h>
#include "framework.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum MsgType
    {
        Information,
        Warning,
        Error,
        LogMsg
    };

    explicit MainWindow(const char* titleText);
    ~MainWindow() override;
    void setTitle(QString titleText);
    
public slots:
    void Msg(const QString& msg, MainWindow::MsgType msgType = Information);
private:
    void LoadAppConfig();
    void LoadGameConfig();
    void InitializeGUI();

    void RegisterHotKeys();
    
    QHash<QString,bool> cfg;
    DWORD  pid;
    quint16  int_Counter;
    qint32 int_UpperLimit_FPS;
    bool configured_Memory;
    bool is_ProcessRunning;
    bool is_atMainPage;
    QString             info_Path_Genshin;
    PROCESS_INFORMATION info_Process{};
    framework::MemoryInfo info_Memory{};
    MODULEENTRY32       module_UnityLib{};
    QTimer       *timer_AutoDetect;
    QTimer       *timer_GetModule;
    QTimer       *timer_Monitor;
    QPixmap* img_background;
    MainWindowUI *ui;
private slots:
    void Memory_Initialize();
    void Memory_Configure();

    void DetectGamePath();
    void StartDetectProcess();
    void ProcessMonitor();
    void StartProcess();
    void SwitchPage();

    void ApplyLimit();
    void setLimitMinus();
    void setLimitPlus();
    void SetLimit(qint32 FPS_upperLimitValue);

    void ApplySettings();
    void LoadSettingsToUI();
signals:
    void SIG_Error(QString errMsg);
    void SIG_AppConfigured();
    void MSG(QString msg, MainWindow::MsgType msgType = Information);
protected:
    bool eventFilter(QObject *obj, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
};

#endif // MAINWINDOW_H
