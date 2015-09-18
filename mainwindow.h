#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QAbstractButton>

enum Scaling { ScaledProportions, ScaledCropped, TiledNotScaled, NotScaled };
enum Gravity { North, NorthEast, East, SouthEast, South, SouthWest, West,
               NorthWest, Center };
enum Folder { ConfigFolder, ShmFolder, TmpFolder };

struct dialogdata {
    QString listfile;
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

    void on_listfileBrowse_clicked();
    void listfileBrowseDialog_selected(const QString &selected);

    void on_bgcolorSelect_clicked();
    void bgcolorSelectDialog_selected(const QColor &color);

    void on_bgcolor_textChanged(const QString &arg1);

    void on_targetScreen_clicked();

    void on_targetDesktop_clicked();

private:
    Ui::MainWindow *ui;

    void updateBgcolorWidgetSheet();
};

#endif // MAINWINDOW_H
