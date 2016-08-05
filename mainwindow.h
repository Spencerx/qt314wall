#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QAbstractButton>

enum Source { ImageSource, ListSource, FolderSource };
enum Scaling { ScaledProportions, ScaledCropped, TiledNotScaled, NotScaled };
enum Gravity { North, NorthEast, East, SouthEast, South, SouthWest, West,
               NorthWest, Center };
enum Folder { ConfigFolder, ShmFolder, TmpFolder };

struct dialogdata {
    Source source;
    QString image;
    QString listfile;
    QString fileFolder;
    int hr, mn, sc;
    QColor bgcolor;
    bool multiply;
    Scaling scale;
    Gravity weight;
    Folder folder;
    bool running;
    QSize target;
    bool xsetbg;

    dialogdata() : listfile(), hr(0), mn(0), sc(10), bgcolor("#303030"),
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

    void setData(const dialogdata &d);

signals:
    void dataChanged(const dialogdata &d);

private slots:

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_imageBrowse_clicked();

    void on_listfileBrowse_clicked();

    void on_fileFolderBrowse_clicked();

    void on_bgcolorSelect_clicked();

    void on_bgcolor_textChanged(const QString &arg1);

    void on_targetScreen_clicked();

    void on_targetDesktop_clicked();

private:
    Ui::MainWindow *ui;

    void updateBgcolorWidgetSheet();
};

#endif // MAINWINDOW_H
