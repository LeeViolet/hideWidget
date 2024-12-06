#ifndef MYGLOBALSHORTCUT_H
#define MYGLOBALSHORTCUT_H

#include <QApplication>
#include <windows.h>
#include <QKeySequence>
#include <QHash>
#include "MyWinEventFilter.h"

class MyGlobalShortCut : public QObject
{
    Q_OBJECT
public:
    MyGlobalShortCut(QString key, QObject *app);
    ~MyGlobalShortCut();
    void activateShortcut();
    bool registerHotKey();
    bool unregisterHotKey();
    QHash<QPair<quint32, quint32>, MyGlobalShortCut*> shortcuts;

private:
    QApplication *m_app;
    QKeySequence m_key;
    Qt::Key key;
    Qt::KeyboardModifiers mods;
    MyWinEventFilter *m_filter;
    static quint32 nativeKeycode(Qt::Key keycode);
    static quint32 nativeModifiers(Qt::KeyboardModifiers modifiers);

signals:
    void activated();
};

#endif // MYGLOBALSHORTCUT_H
