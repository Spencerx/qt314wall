#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QApplication>
#include <QDesktopWidget>

const char *dialogdata::gravityStrings[] = {
    "north", "northeast", "east", "southeast", "south", "southwest", "west",
    "northwest", "center"
};

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setData(const dialogdata &d)
{
    ui->listfile->setText(d.listfile);
    ui->hrs->setValue(d.hr);
    ui->min->setValue(d.mn);
    ui->sec->setValue(d.sc);
    ui->bgcolor->setText(d.bgcolor.name());
    ui->multiply->setChecked(d.multiply);
    ui->scale->setCurrentIndex(d.scale);
    ui->gravity->setCurrentIndex(d.weight);
    ui->folder->setCurrentIndex(d.folder);
    ui->running->setChecked(d.running);
    ui->targetWidth->setValue(d.target.width());
    ui->targetHeight->setValue(d.target.height());
    ui->xsetbg->setChecked(d.xsetbg);
}

void MainWindow::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole br = ui->buttonBox->buttonRole(button);
    if (br == QDialogButtonBox::AcceptRole || br == QDialogButtonBox::ApplyRole) {
        dialogdata d;
        d.listfile = ui->listfile->text();
        d.hr = ui->hrs->value();
        d.mn = ui->min->value();
        d.sc = ui->sec->value();
        d.bgcolor = ui->bgcolor->text();
        d.multiply = ui->multiply->isChecked();
        d.scale = static_cast<Scaling>(ui->scale->currentIndex());
        d.weight = static_cast<Gravity>(ui->gravity->currentIndex());
        d.folder = static_cast<Folder>(ui->folder->currentIndex());
        d.running = ui->running->isChecked();
        d.target = QSize(ui->targetWidth->value(), ui->targetHeight->value());
        d.xsetbg = ui->xsetbg->isChecked();
        emit dataChanged(d);
    }
    if (br == QDialogButtonBox::AcceptRole || br == QDialogButtonBox::RejectRole) {
        this->hide();
    }
}

void MainWindow::on_listfileBrowse_clicked()
{
    QFileDialog *qfd = new QFileDialog(this);
    qfd->setWindowTitle(tr("Open Text File"));
    qfd->selectFile(ui->listfile->text());
    connect(qfd, &QFileDialog::fileSelected,
            this, &MainWindow::listfileBrowseDialog_selected);
    qfd->show();
}

void MainWindow::listfileBrowseDialog_selected(const QString &selected)
{
    ui->listfile->setText(selected);
}

void MainWindow::on_bgcolorSelect_clicked()
{
    QColorDialog *qcd = new QColorDialog(this);
    qcd->setCurrentColor(QColor(ui->bgcolor->text()));
    connect(qcd, &QColorDialog::colorSelected,
            this, &MainWindow::bgcolorSelectDialog_selected);
    qcd->show();
}

void MainWindow::bgcolorSelectDialog_selected(const QColor &color)
{
    ui->bgcolor->setText(color.name());
    ui->bgcolor->setFocus();
    updateBgcolorWidgetSheet();
}

void MainWindow::on_bgcolor_textChanged(const QString &arg1)
{
    (void)arg1;
    updateBgcolorWidgetSheet();
}

void MainWindow::updateBgcolorWidgetSheet()
{
    ui->bgcolorSelect->setStyleSheet(QString("background: %1").arg(ui->bgcolor->text()));
}

void MainWindow::on_targetScreen_clicked()
{
    QDesktopWidget *desktop = qApp->desktop();
    QRect r = desktop->screenGeometry(desktop->screenNumber(QCursor::pos()));
    ui->targetWidth->setValue(r.width());
    ui->targetHeight->setValue(r.height());
}

void MainWindow::on_targetDesktop_clicked()
{
    QDesktopWidget *desktop = qApp->desktop();
    QSize r = desktop->size();
    ui->targetWidth->setValue(r.width());
    ui->targetHeight->setValue(r.height());
}
