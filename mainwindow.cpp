#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QListWidgetItem>

QStringList MainWindow::files = QStringList();
std::vector<HWND> MainWindow::handlers = std::vector<HWND>();
QStringListModel *MainWindow::m_model = nullptr;


WINBOOL MainWindow::EnumAllWindows(HWND hwnd, LPARAM)
{
    DWORD processId;
    if (GetWindowThreadProcessId(hwnd, &processId) == 0)
    {
        qWarning() << "Get Process Id fail";
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        qWarning() << "Failed to open process";
        return false;
    }

    // Using QueryFullProcessImageName as it is recommended over GetModuleFileNameEx
    wchar_t filePath[MAX_PATH];
    DWORD size = MAX_PATH;
    if (!QueryFullProcessImageName(hProcess, 0, filePath, &size)) {
        qWarning() << "Failed to query full process image name";
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    QString name = QString::fromWCharArray(filePath);
    for (const QString &fileName : m_model->stringList())
    {
        if (IsWindowVisible(hwnd) && fileName == name)
        {
            handlers.push_back(hwnd);
            ShowWindow(hwnd, SW_HIDE);
        }
    }
    return true;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建托盘图标
    m_sysTrayIcon = new QSystemTrayIcon(this);
    QIcon icon = QIcon(":/ico/H.ico");
    m_sysTrayIcon->setIcon(icon);
    m_sysTrayIcon->setToolTip("H");
    QMenu *m_menu = new QMenu(this);
    QAction *m_showMainAction = new QAction("显示主窗口", this);
    QAction *m_exitAppAction = new QAction("退出程序", this);
    m_menu->addAction(m_showMainAction);
    m_menu->addSeparator();
    m_menu->addAction(m_exitAppAction);
    m_sysTrayIcon->setContextMenu(m_menu);
    connect(m_sysTrayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            this->show();
        } else if (reason == QSystemTrayIcon::Trigger) {
            // 单击托盘图标时的操作
            this->show();
        }
    });
    connect(m_showMainAction, &QAction::triggered, this, &MainWindow::show);
    // connect(m_exitAppAction, &QAction::triggered, this, &MainWindow::close);
    connect(m_exitAppAction, SIGNAL(triggered()), this, SLOT(do_exit()));
    m_sysTrayIcon->show();

    connect(ui->btnChoseExe, SIGNAL(clicked()), this, SLOT(do_chooseExe()));
    connect(ui->btnSetHotKey, SIGNAL(clicked()), this, SLOT(do_setHotKey()));
    readIni();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_model;
    if (!handlers.empty())
    {
        for (auto it = handlers.begin(); it != handlers.end(); ++it)
        {
            ShowWindow(*it, SW_SHOW);
        }
        handlers.clear();
    }
}

void MainWindow::rec_testShotCutAns()
{
    if (handlers.size() != 0)
    {
        for (auto it = handlers.begin(); it != handlers.end(); ++it)
        {
            ShowWindow(*it, SW_SHOW);
        }
        handlers.clear();
    }
    else if (m_model->stringList().size())
    {
        LPARAM p = 0;
        EnumWindows(EnumAllWindows, p);
    }
}

// 选择程序
void MainWindow::do_chooseExe()
{
    QString name = QFileDialog::getOpenFileName(this, "选择应用程序", QDir::rootPath(), "应用程序 (*.exe)");
    if (name.isEmpty()) return;
    name = name.replace("/", "\\");
    // 检查是否已选择程序
    QStringList list = m_model->stringList();
    if (list.contains(name)) return;
    m_model->insertRow(m_model->rowCount());
    QModelIndex index = m_model->index(m_model->rowCount()-1,0);
    m_model->setData(index, name, Qt::DisplayRole);
    ui->listView->setCurrentIndex(index);
    // 保存 ini 文件
    QSettings *setting = new QSettings("config.ini", QSettings::IniFormat);
    setting->setValue("ini/files", m_model->stringList());
    delete setting;
    if (!globalShortCut) setConnect();
}

void MainWindow::readIni()
{
    QSettings *setting = new QSettings("config.ini", QSettings::IniFormat);
    QString hot = setting->value("ini/key").toString();
    if (!hot.isEmpty())
    {
        hotKey = hot;
        ui->keySequenceEdit->setKeySequence(hotKey);
    }
    files = setting->value("ini/files").toStringList();
    // 模型-视图
    m_model = new QStringListModel(this);
    m_model->setStringList(files);
    ui->listView->setModel(m_model);
    delete setting;
    if (setConnect())
    {
        this->hide();
    }
    else
    {
        this->show();
    }
}

void MainWindow::do_setHotKey()
{
    QKeySequence seq = ui->keySequenceEdit->keySequence();
    QString key = seq.toString();
    if (key == hotKey || key.isEmpty()) return;
    QSettings *setting = new QSettings("config.ini", QSettings::IniFormat);
    setting->setValue("ini/key", key);
    hotKey = key;

    // 重新设置槽函数
    setConnect();
}

void MainWindow::on_btnMini_clicked()
{
    this->hide();
}

bool MainWindow::setConnect()
{
    if (!m_model->stringList().count() || hotKey.isEmpty()) return false;
    if (globalShortCut != nullptr)
    {
        disconnect(globalShortCut);
        delete globalShortCut;
    }
    globalShortCut = new MyGlobalShortCut(hotKey, this);
    QObject::connect(globalShortCut, SIGNAL(activated()), this, SLOT(rec_testShotCutAns()));
    ui->labStatus->setText("已启动");
    return true;
}

void MainWindow::do_exit()
{
    isTray = true;
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isTray)
    {
        event->accept();
        QApplication::quit();
        return;
    }
    QString dlgTitle = "消息框";
    QString strInfo = "确定要退出吗？";
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(dlgTitle);
    msgBox.setText(strInfo);
    QAbstractButton *closeBtn = msgBox.addButton("退出", QMessageBox::YesRole);
    msgBox.addButton("取消", QMessageBox::NoRole);
    QAbstractButton *miniBtn = msgBox.addButton("最小化", QMessageBox::ApplyRole);
    msgBox.exec();
    if (msgBox.clickedButton() == closeBtn)
    {
        event->accept();
        // QApplication::quit();
        return;
    }
    else if (msgBox.clickedButton() == miniBtn)
    {
        this->hide();
    }
    event->ignore();
}


void MainWindow::on_btnRemoveExe_clicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (index.row() == -1) return;
    m_model->removeRow(index.row());
    QSettings *setting = new QSettings("config.ini", QSettings::IniFormat);
    setting->setValue("ini/files", m_model->stringList());
    delete setting;
}


void MainWindow::on_btnClearExe_clicked()
{
    m_model->removeRows(0, m_model->rowCount());
    QSettings *setting = new QSettings("config.ini", QSettings::IniFormat);
    setting->remove("ini/files");
    delete setting;
}

