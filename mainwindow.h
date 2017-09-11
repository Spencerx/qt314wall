#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QAbstractButton>
#include <QLineEdit>
#include "source.h"

enum Source { ImageSource, ListSource, FolderSource, DropSource, WebSource };
enum Scaling { ScaledProportions, ScaledCropped, TiledNotScaled, NotScaled };
enum Gravity { North, NorthEast, East, SouthEast, South, SouthWest, West,
               NorthWest, Center };
enum Folder { ConfigFolder, ShmFolder, TmpFolder };

struct dialogdata {
    Source source;
    QString image;
    QString listfile;
    QString fileFolder;
    QStringList droppedFiles;
    QStringList webFields;
    int webIndex;
    int hr, mn, sc;
    QColor bgcolor;
    bool multiply;
    Scaling scale;
    Gravity weight;
    Folder folder;
    bool running;
    QSize target;
    bool xsetbg;
    bool plasmaDBus;

    dialogdata() : listfile(), hr(0), mn(0), sc(10), bgcolor(48,48,48),
        multiply(true), scale(ScaledProportions), weight(SouthEast) { }
    static const char *gravityStrings[];
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setWebSources(const QList<Sources::WebSource*> &sources);
    void setData(const dialogdata &d);
    void setRunning(bool running);

signals:
    void dataChanged(const dialogdata &d);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void setupSources();

private slots:

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_bgcolorSelect_clicked();

    void on_bgcolor_textChanged(const QString &arg1);

    void on_targetScreen_clicked();

    void on_targetDesktop_clicked();

    void on_webSource_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    void updateBgcolorWidgetSheet();
    QStringList lastDroppedFiles;
    QStringList webFields;
    QList<QLineEdit*> webFieldWidgets;
};

#endif // MAINWINDOW_H
