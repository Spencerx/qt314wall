#include "main.h"
#include <QApplication>
#include <QSettings>
#include <QLockFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QDebug>

static QString configFolder;

int main(int argc, char *argv[])
{
    QApplication::setOrganizationDomain("cmdrkotori.github.com");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication a(argc, argv);

    // one instance only!
    configFolder = QFileInfo(QSettings("qt314wall", "qt314wall").fileName()).absolutePath() + "/";
    QLockFile lock("/dev/shm/qt314wall.lockfile");
    if (!lock.tryLock()) {
        QMessageBox::critical(NULL, QObject::tr("Qt314Wall"), QObject::tr("Application already running (Delete lockfile?)"));
        return 1;
    }

    // prep slideshow directories
    QDir("/dev/shm").mkdir("qt314-wallpaper");
    QDir("/tmp").mkdir("qt314-wallpaper");

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
    sysicon->setIcon(QIcon(":/images/clock.png"));
    sysicon->setToolTip(tr("Cutie-pie Wallpaper Changer"));

    ctxmenu = new QMenu();

    a = new QAction(this);
    a->setText("Show");
    connect(a, &QAction::triggered, this, &Flow::show_triggered);
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
    if (ctxmenu)    delete ctxmenu;
    if (sysicon)    delete sysicon;
    if (window)     delete window;
    if (timer)      delete timer;
}

void Flow::run()
{
    fetchSettings();
    updateItems();
    updateTimerInterval();
    updateDestFolder();
    updateTargetString();
    window->setData(settings);
    sysicon->show();
    changeOneWall();
}

void Flow::show_triggered()
{
    window->showNormal();
    window->activateWindow();
}

void Flow::dialogDataChanged(const dialogdata &d)
{
    settings = d;
    storeSettings();
    updateTimerInterval();
    updateDestFolder();
    updateTargetString();
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
    QFile org("/dev/shm/qt314wall-tempimage.png");
    auto rmcp = [&](const char *name) {
        QFile(this->destfolder + name).remove();
        org.copy(this->destfolder + name);
    };

    if (!exitCode) {
        rmcp("image.png");
        rmcp("image2.png");
        org.remove();
        if (settings.xsetbg) {
            QProcess::startDetached("xsetbg", QStringList() <<
                                    this->destfolder + "image.png");
        }
    } else {
        qDebug() << converter->readAllStandardError();
    }
}

void Flow::storeSettings()
{
    QSettings s("qt314wall", "qt314wall");
    s.setValue("listfile", settings.listfile);
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
}

void Flow::fetchSettings()
{
    QSettings s("qt314wall", "qt314wall");
    settings.listfile = s.value("listfile").toString();
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
}

void Flow::updateItems()
{
    QFile f(settings.listfile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    items.clear();
    while (!f.atEnd()) {
        items.append(QString::fromUtf8(f.readLine()).trimmed());
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
        destfolder = configFolder;
        break;
    case ShmFolder:
        destfolder = "/dev/shm/qt314-wallpaper/";
        break;
    case TmpFolder:
    default:
        destfolder = "/tmp/qt314-wallpaper/";
    }
}

void Flow::updateTargetString()
{
    targetString = QString("%1x%2").arg(settings.target.width())
            .arg(settings.target.height());
}

void Flow::changeOneWall()
{
    if (items.isEmpty()) {
        qDebug() << "empty items";
        return;
    }
    std::uniform_int_distribution<int> dist(0, items.size()-1);
    int index = dist(rgen);
    QString srcfname = items.at(index);
    QStringList args;
    // convert to linear space
    args << srcfname << "-colorspace" << "RGB";
    switch (settings.scale) {
    case ScaledProportions:
        // scale to fit
        args << "-resize" << targetString
             << "-size" << targetString
             << "-gravity" << dialogdata::gravityStrings[settings.weight];
        break;
    case ScaledCropped:
        // scale to cover
        args << "-resize" << targetString + "^"
             << "-gravity" << "center"
             << "-crop" << targetString + "+0+0";
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
