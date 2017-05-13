#ifndef MAIN_H
#define MAIN_H
#include <QSystemTrayIcon>
#include <QLocalServer>
#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QLockFile>
#include <QTimer>
#include <QProcess>
#include <ext/random>
#include "mainwindow.h"

class Flow : public QObject {
    Q_OBJECT
public:
    Flow(QObject *parent = NULL);
    ~Flow();

    static bool passToPrevious(const QStringList &files);
    void run();
    void removeActiveFile();

private slots:
    void server_newConnection();
    void show_triggered();
    void enabled_toggled(bool state);
    void openImage_triggered();
    void nextImage_triggered();
    void dialogDataChanged(const dialogdata &d);
    void changeWall();
    void changeWallConvertFinished(int exitCode);

private:
    MainWindow *window;
    QSystemTrayIcon *sysicon;
    QLocalServer server;
    QMenu *ctxmenu;
    QTimer *timer;
    QProcess *converter;
    QAction *enableAction;
    dialogdata settings;
    QStringList items;
    QString destfolder;
    QString targetString;
    QString activeFilename;
    QString activeSource;
    std::random_device rseed;
    std::mt19937 rgen;

    void setupServer();
    bool maybeSetToFiles(const QStringList &candidates, const QString &workingFolder = QString());
    void storeSettings();
    void fetchSettings();

    void updateItems();
    void updateTimerInterval();
    void updateDestFolder();
    void updateTargetString();
    void updateEnabled();
    void changeOneWall();

    QString calcTileSize(const QString &srcfname);
};


#endif // MAIN_H
