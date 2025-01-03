#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include "MyGlobalShortCut.h"
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QStringListModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    WINBOOL static EnumAllWindows(HWND hwnd, LPARAM p);
    std::vector<HWND> static handlers;
    QStringListModel static *m_model;
    QStringListModel static *temp_model;

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    void readIni();
    QString hotKey = "";
    MyGlobalShortCut *globalShortCut = nullptr;
    QSystemTrayIcon *m_sysTrayIcon;
    bool setConnect();
    bool isTray = false;

private slots:
    void rec_testShotCutAns();
    void do_chooseExe();
    void do_setHotKey();
    void do_exit();
    void on_btnMini_clicked();
    void on_btnRemoveExe_clicked();
    void on_btnClearExe_clicked();
    void on_btnUp_clicked();
    void on_btnDown_clicked();
};


#endif // MAINWINDOW_H
