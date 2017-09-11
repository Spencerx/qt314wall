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
#include "source.h"

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
    void enabled_triggered(bool state);
    void openImage_triggered();
    void openSource_triggered();
    void nextImage_triggered();
    void dialogDataChanged(const dialogdata &d);
    void source_nextFile(QString file);
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
    QString item;
    QString destfolder;
    QString targetString;
    QString generatedFilename;
    QString activeSourceFilename;
    std::random_device rseed;
    std::mt19937 rgen;

    bool requestingSource;
    Sources::FileSource *activeSource;
    Sources::FileSource *fileSource;
    Sources::FileListSource *fileListSource;
    Sources::FolderSource *folderSource;
    Sources::DropSource *dropSource;
    QList<Sources::WebSource*> webSources;

    void setupSources();
    void setupServer();
    bool maybeSetToFiles(const QStringList &candidates, const QString &workingFolder = QString());
    void storeSettings();
    void fetchSettings();

    void requestNextImage();
    void updateTimerInterval();
    void updateDestFolder();
    void updateTargetString();
    void updateEnabled();
    void updateSources();
    void changeOneWall();

    QString calcTileSize(const QString &srcfname);
};


#endif // MAIN_H
