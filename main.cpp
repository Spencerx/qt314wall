#include "main.h"
#include <QApplication>
#include <QSettings>
#include <QLockFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QUuid>
#include <QDBusInterface>
#include <QLocalSocket>
#include <QUrl>

static QString configFolderPath;
static const char serverName[] = "cmdrkotori.qt314wall";
static const int serverTimeout = 1000;
static const char orgDomain[] = "cmdrkotori.github.com";
static const char configFolderTitle[] = "qt314wall";
static const char workingDirNameShm[] = "/dev/shm/qt314-wallpaper";
static const char workingDirNameTmp[] = "/tmp/qt314-wallpaper";

int main(int argc, char *argv[])
{
    QApplication::setOrganizationDomain(orgDomain);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication a(argc, argv);
    if (Flow::passToPrevious(a.arguments().mid(1)))
        return 0;

    configFolderPath = QFileInfo(QSettings(configFolderTitle, configFolderTitle).fileName()).absolutePath() + "/";

    // prep slideshow directories
    QDir("/").mkpath(workingDirNameShm);
    QDir("/").mkpath(workingDirNameTmp);

    Flow f;
    f.run();
    return a.exec();
}


Flow::Flow(QObject *parent) : QObject(parent),
    window(NULL), sysicon(NULL), ctxmenu(NULL), timer(NULL),
    converter(NULL), rgen(rseed())
{
    QAction *a;

    sysicon = new QSystemTrayIcon(this);
    sysicon->setIcon(QIcon(":/images/clock2.png"));
    sysicon->setToolTip(tr("Cutie-pie Wallpaper Changer"));

    ctxmenu = new QMenu();

    a = new QAction(this);
    a->setText(tr("Show"));
    connect(a, &QAction::triggered, this, &Flow::show_triggered);
    ctxmenu->addAction(a);

    enableAction = new QAction(this);
    enableAction->setText(tr("Enable"));
    enableAction->setCheckable(true);
    connect(enableAction, &QAction::toggled, this, &Flow::enabled_toggled);
    ctxmenu->addAction(enableAction);

    a = new QAction(this);
    a->setText(tr("Next Image"));
    connect(a, &QAction::triggered, this, &Flow::nextImage_triggered);
    ctxmenu->addAction(a);

    ctxmenu->addSeparator();

    a = new QAction(this);
    a->setText(tr("Open Image"));
    connect(a, &QAction::triggered, this, &Flow::openImage_triggered);
    ctxmenu->addAction(a);

    ctxmenu->addSeparator();

    a = new QAction(this);
    a->setText(tr("Exit"));
    connect(a, &QAction::triggered, qApp, &QApplication::quit);
    ctxmenu->addAction(a);

    sysicon->setContextMenu(ctxmenu);

    window = new MainWindow();
    connect(window, &MainWindow::dataChanged, this, &Flow::dialogDataChanged);

    converter = new QProcess(this);
    connect(converter, SIGNAL(finished(int)), this, SLOT(changeWallConvertFinished(int)));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Flow::changeWall);
    timer->start(10000);
}

Flow::~Flow()
{
    removeActiveFile();
    if (ctxmenu)    delete ctxmenu;
    if (sysicon)    delete sysicon;
    if (window)     delete window;
    if (timer)      delete timer;
}

bool Flow::passToPrevious(const QStringList &files)
{
    QLocalSocket sock;
    sock.setServerName(serverName);
    sock.connectToServer(QIODevice::ReadWrite | QIODevice::Text);
    if (!sock.waitForConnected(serverTimeout)) {
        return false;
    }
    QStringList toSend;
    toSend.append(QDir::currentPath());
    toSend.append(files);
    sock.write(toSend.join('\n').toUtf8());
    return sock.waitForBytesWritten(serverTimeout);
}

void Flow::run()
{
    setupServer();
    fetchSettings();
    if (QApplication::arguments().count() > 1)
        maybeSetToFiles(QApplication::arguments().mid(1), QDir::currentPath());
    updateItems();
    updateTimerInterval();
    updateDestFolder();
    updateTargetString();
    updateEnabled();
    window->setData(settings);
    sysicon->show();
    changeOneWall();
}

void Flow::removeActiveFile()
{
    QFile(this->destfolder + activeFilename).remove();
}

void Flow::server_newConnection()
{
    QLocalSocket *sock = server.nextPendingConnection();
    if (!sock)
        return;
    sock->waitForReadyRead(serverTimeout * 2);
    QStringList lines = QString::fromUtf8(sock->readAll()).split('\n');
    delete sock;

    if (lines.count() < 2) {
        window->show();
        window->activateWindow();
        return;
    }
    if (!maybeSetToFiles(lines.mid(1), lines.first()))
        return;

    window->setData(settings);
    dialogDataChanged(settings);
}

void Flow::show_triggered()
{
    window->setData(settings);
    window->showNormal();
    window->activateWindow();
}

void Flow::enabled_toggled(bool state)
{
    settings.running = state;
    storeSettings();
    if (state) {
        updateTimerInterval();
        changeOneWall();
    } else {
        timer->stop();
    }
}

void Flow::openImage_triggered()
{
    QProcess::startDetached("xdg-open", QStringList() << activeSource);
}

void Flow::nextImage_triggered()
{
    changeOneWall();
}

void Flow::dialogDataChanged(const dialogdata &d)
{
    settings = d;
    storeSettings();
    updateItems();
    updateTimerInterval();
    updateDestFolder();
    updateTargetString();
    updateEnabled();
    changeOneWall();
}

void Flow::changeWall()
{
    if (!settings.running)
        return;
    changeOneWall();
}

void Flow::changeWallConvertFinished(int exitCode)
{
    if (exitCode) {
        qDebug() << converter->readAllStandardError();
        return;
    }

    QFile org("/dev/shm/qt314wall-tempimage.png");
    std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)-1ll);
    QString filename = QString("%1.png").arg(dist(rgen));
    org.copy(this->destfolder + filename);
    removeActiveFile();
    org.remove();
    activeFilename = filename;
    if (settings.xsetbg)
        QProcess::startDetached("xsetbg", QStringList() <<
                                this->destfolder + activeFilename);
    if (settings.plasmaDBus) {
        QDBusInterface plasma("org.kde.plasmashell",
                              "/PlasmaShell",
                              "org.kde.PlasmaShell");
        if (plasma.isValid()) {
            QString file(destfolder + activeFilename);
            file.replace('"', "\\\"");
            QFile scriptFile(":/text/plasmascript.txt");
            scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QString script = QString::fromLocal8Bit(scriptFile.readAll()).arg(file);
            plasma.call("evaluateScript", script);
        }
    }
}

void Flow::setupServer()
{
    connect(&server, &QLocalServer::newConnection, this, &Flow::server_newConnection);
    server.listen(serverName);
}

bool Flow::maybeSetToFiles(const QStringList &candidates, const QString &workingFolder)
{
    QStringList files;
    QStringList validExtensions({ "jpg", "jpeg", "jpe", "png", "bmp", "dib", "gif" });
    for (const QString &s : candidates) {
        QString filename = QUrl::fromUserInput(s, workingFolder).toLocalFile();
        QFileInfo fileinfo(filename);
        if (fileinfo.exists() && validExtensions.contains(fileinfo.suffix().toLower()))
            files.append(filename);
    }
    if (files.count() < 1)
        return false;
    settings.droppedFiles = files;
    settings.source = DropSource;
    return true;

}

void Flow::storeSettings()
{
    QSettings s("qt314wall", "qt314wall");
    s.setValue("source", settings.source);
    s.setValue("image", settings.image);
    s.setValue("listfile", settings.listfile);
    s.setValue("filefolder", settings.fileFolder);
    s.setValue("droppedfiles", settings.droppedFiles);
    s.setValue("hours", settings.hr);
    s.setValue("minutes", settings.mn);
    s.setValue("seconds", settings.sc);
    s.setValue("bgcolor", settings.bgcolor.name());
    s.setValue("multiply", settings.multiply);
    s.setValue("scale", settings.scale);
    s.setValue("weight", settings.weight);
    s.setValue("folder", settings.folder);
    s.setValue("running", settings.running);
    s.setValue("target", settings.target);
    s.setValue("xsetbg", settings.xsetbg);
    s.setValue("plasmadbus", settings.plasmaDBus);
    s.sync();
}

void Flow::fetchSettings()
{
    QSettings s("qt314wall", "qt314wall");
    settings.source = static_cast<Source>(s.value("source", ImageSource).toInt());
    settings.image = s.value("image").toString();
    settings.listfile = s.value("listfile").toString();
    settings.fileFolder = s.value("filefolder").toString();
    settings.droppedFiles = s.value("droppedfiles").toStringList();
    settings.hr = s.value("hours").toInt();
    settings.mn = s.value("minutes").toInt();
    settings.sc = s.value("seconds", 10).toInt();
    settings.bgcolor.setNamedColor(s.value("bgcolor", "#303030").toString());
    settings.multiply = s.value("multiply", true).toBool();
    settings.scale = static_cast<Scaling>(s.value("scale",ScaledProportions).toInt());
    settings.weight = static_cast<Gravity>(s.value("weight", SouthEast).toInt());
    settings.folder = static_cast<Folder>(s.value("folder", ShmFolder).toInt());
    settings.running = s.value("running", false).toBool();
    settings.target = s.value("target", QSize(1920,1080)).toSize();
    settings.xsetbg = s.value("xsetbg", false).toBool();
    settings.plasmaDBus = s.value("plasmadbus", true).toBool();
}

void Flow::updateItems()
{
    switch (settings.source) {
    case ImageSource: {
        items.clear();
        items.append(settings.image);
        break;
    }
    case ListSource: {
        QFile f(settings.listfile);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        items.clear();
        while (!f.atEnd()) {
            items.append(QString::fromUtf8(f.readLine()).trimmed());
        }
        break;
    }
    case FolderSource: {
        QDir d(settings.fileFolder);
        items.clear();
        for (auto i : d.entryInfoList({"*.jpg", "*.png"}, QDir::Files))
            items.append(i.absoluteFilePath());
        break;
    }
    case DropSource: {
        items = settings.droppedFiles;
        break;
    }
    }
}

void Flow::updateTimerInterval()
{
    timer->stop();
    timer->start(std::max(10000, (settings.hr*3600000) + (settings.mn*60000) + (settings.sc*1000)));
}

void Flow::updateDestFolder()
{
    switch (settings.folder) {
    case ConfigFolder:
        destfolder = configFolderPath;
        break;
    case ShmFolder:
        destfolder = workingDirNameShm;
        break;
    case TmpFolder:
    default:
        destfolder = workingDirNameTmp;
    }
}

void Flow::updateTargetString()
{
    targetString = QString("%1x%2").arg(settings.target.width())
            .arg(settings.target.height());
}

void Flow::updateEnabled()
{
    enableAction->setChecked(settings.running);
}

void Flow::changeOneWall()
{
    if (items.isEmpty()) {
        qDebug() << "empty items";
        return;
    }
    std::uniform_int_distribution<int> dist(0, items.size()-1);
    int index = dist(rgen);
    QString srcfname = items.value(index);
    QFileInfo inspector(srcfname);
    if (!inspector.isReadable() || !inspector.isFile())
        return;
    activeSource = srcfname;
    QString rs(targetString);
    rs.append("^");
    QString rs2(targetString);
    rs2.append("+0+0");
    // convert to linear space
    QStringList args;
    args << activeSource << "-colorspace" << "RGB";
    switch (settings.scale) {
    case ScaledProportions:
        // scale to fit
        args << "-resize" << targetString
             << "-size" << targetString
             << "-gravity" << dialogdata::gravityStrings[settings.weight];
        break;
    case ScaledCropped:
        // scale to cover
        args << "-resize" << rs
             << "-gravity" << "center"
             << "-crop" << rs2
             << "-write" << "mpr:src" << "+delete"
             << "-background" << "rgba(255,255,255)"
             << "-size" << targetString
             << "mpr:src";
        break;
    case TiledNotScaled:
        // tile oversize for screen, then crop  (fairly expensive tbh)
        args << "-write" << "mpr:src" << "+delete"
             << "-background" << "rgba(0,0,0,0)"
             << "-size" << calcTileSize(srcfname)
             << "tile:mpr:src"
             << "-gravity" << dialogdata::gravityStrings[settings.weight]
             << "-size" << targetString;
        break;
    case NotScaled:
    default:
        // place at corner
        args << "-size" << targetString
             << "-gravity" << dialogdata::gravityStrings[settings.weight];
    }
    // convert to monitor space
    args << "-colorspace" << "sRGB";
    if (settings.multiply)      // dither before multiply to reduce banding
        args << "-ordered-dither" << QString("8x8,%1,%1,%1").arg(
                    std::max(std::max(settings.bgcolor.red(),
                                      settings.bgcolor.blue()),
                             settings.bgcolor.green()));
    // create background
    args << QString("xc:%1").arg(settings.bgcolor.name()) << "+swap";
    if (settings.multiply)      // apply screen
        args << "-compose" << "multiply";
    // make wall
    args << "-composite" << "/dev/shm/qt314wall-tempimage.png";
    converter->start("convert", args);
}

QString Flow::calcTileSize(const QString &srcfname)
{
    QProcess p;
    p.start("identify", QStringList() << "-format" << "%w\t%h" << srcfname);
    p.waitForFinished();
    QStringList qsl = QString::fromUtf8(p.readLine()).trimmed().split("\t");
    if (qsl.count() < 2) {
        return targetString;
    }
    int w = qsl.at(0).toInt();
    int h = qsl.at(1).toInt();

    int tx = ((settings.target.width() / w) + 1) | 1;
    int ty = ((settings.target.height() / h) + 1) | 1;

    int ox = w*tx;
    int oy = h*ty;

    return QString("%1x%2").arg(ox).arg(oy);
}
