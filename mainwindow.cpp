#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QApplication>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QSystemTrayIcon>
#include <QDialogButtonBox>
#include "source.h"

const char *dialogdata::gravityStrings[] = {
    "north", "northeast", "east", "southeast", "south", "southwest", "west",
    "northwest", "center"
};

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupSources();
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        auto cancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
        ui->buttonBox->removeButton(cancel);
        delete cancel;
        ui->buttonBox->addButton(QDialogButtonBox::Close);
        connect(ui->buttonBox, &QDialogButtonBox::rejected,
                qApp, &QApplication::quit);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setWebSources(const QList<Sources::WebSource *> &sources)
{
    int i = 0;
    for (auto s : sources) {
        QString fieldText = s->tags().join(' ');
        ui->webSource->addItem(s->title());
        QLineEdit *edit = new QLineEdit(this);
        edit->setText(fieldText);
        ui->webSourcePages->addWidget(edit);
        connect(edit, &QLineEdit::textChanged,
                this, [i,this](QString text) {
            webFields[i] = text;
        });
        webFields.append(fieldText);
        webFieldWidgets.append(edit);
        i++;
    }
}

void MainWindow::setData(const dialogdata &d)
{
    ui->sourceImage->setChecked(d.source == ImageSource);
    ui->sourceImageList->setChecked(d.source == ListSource);
    ui->sourceFolder->setChecked(d.source == FolderSource);
    ui->sourceDrop->setChecked(d.source == DropSource);
    ui->sourceWeb->setChecked(d.source == WebSource);
    ui->image->setText(d.image);
    ui->listfile->setText(d.listfile);
    ui->fileFolder->setText(d.fileFolder);
    lastDroppedFiles = d.droppedFiles;
    for (int i = 0; i < webFields.count(); i++) {
        webFields[i] = d.webFields.value(i);
        webFieldWidgets[i]->setText(d.webFields.value(i));
    }
    ui->webSource->setCurrentIndex(d.webIndex);
    ui->hrs->setValue(d.hr);
    ui->min->setValue(d.mn);
    ui->sec->setValue(d.sc);
    ui->bgcolor->setText(d.bgcolor.name());
    ui->multiply->setChecked(d.multiply);
    ui->scale->setCurrentIndex(d.scale);
    ui->gravity->setCurrentIndex(d.weight);
    ui->folder->setCurrentIndex(d.folder);
    ui->targetWidth->setValue(d.target.width());
    ui->targetHeight->setValue(d.target.height());
    ui->initOnce->setChecked(d.initOnce);
    ui->running->setChecked(d.running);
    ui->xsetbg->setChecked(d.xsetbg);
    ui->plasmaDBus->setChecked(d.plasmaDBus);
    updateBgcolorWidgetSheet();
}

void MainWindow::setRunning(bool running)
{
    ui->running->setEnabled(running);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls())
        return;

    lastDroppedFiles.clear();
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.isLocalFile())
            lastDroppedFiles.append(url.toLocalFile());
    }
    ui->sourceDrop->setChecked(true);
}

void MainWindow::setupSources()
{
    struct textfieldSource {
        QString dialogTitle;
        QFileDialog::FileMode fileMode;
        QLineEdit *edit;
        QPushButton *button;
    };

    QList<textfieldSource> fileSources {
        { tr("Open Image"), QFileDialog::ExistingFile,
            ui->image, ui->imageBrowse },
        { tr("Open Text File"), QFileDialog::ExistingFile,
            ui->listfile, ui->listfileBrowse },
        { tr("Open Folder"), QFileDialog::Directory,
            ui->fileFolder, ui->fileFolderBrowse }
    };

    for (auto &s : fileSources) {
        connect(s.button, &QPushButton::clicked,
                this, [s,this]() {
            QFileDialog *qfd = new QFileDialog(this);
            qfd->setFileMode(s.fileMode);
            qfd->setAttribute(Qt::WA_DeleteOnClose);
            qfd->setWindowTitle(s.dialogTitle);
            qfd->selectFile(s.edit->text());
            connect(qfd, &QFileDialog::fileSelected,
                    s.edit, [edit=s.edit](const QString &selected) {
                edit->setText(selected);
            });
            qfd->show();
        });
    }
}

void MainWindow::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole br = ui->buttonBox->buttonRole(button);
    if (br == QDialogButtonBox::AcceptRole || br == QDialogButtonBox::ApplyRole) {
        dialogdata d;
        d.source = ui->sourceImage->isChecked() ? ImageSource :
                   ui->sourceImageList->isChecked() ? ListSource :
                   ui->sourceFolder->isChecked() ? FolderSource :
                   ui->sourceDrop->isChecked() ? DropSource
                                               : WebSource;
        d.image = ui->image->text();
        d.listfile = ui->listfile->text();
        d.fileFolder = ui->fileFolder->text();
        d.droppedFiles = lastDroppedFiles;
        d.webFields = webFields;
        d.webIndex = ui->webSource->currentIndex();
        d.hr = ui->hrs->value();
        d.mn = ui->min->value();
        d.sc = ui->sec->value();
        d.bgcolor = ui->bgcolor->text();
        d.multiply = ui->multiply->isChecked();
        d.scale = static_cast<Scaling>(ui->scale->currentIndex());
        d.weight = static_cast<Gravity>(ui->gravity->currentIndex());
        d.folder = static_cast<Folder>(ui->folder->currentIndex());
        d.initOnce = ui->initOnce->isChecked();
        d.running = ui->running->isChecked();
        d.target = QSize(ui->targetWidth->value(), ui->targetHeight->value());
        d.xsetbg = ui->xsetbg->isChecked();
        d.plasmaDBus = ui->plasmaDBus->isChecked();
        emit dataChanged(d);
    }
    if (br == QDialogButtonBox::AcceptRole || br == QDialogButtonBox::RejectRole) {
        this->hide();
    }
}

void MainWindow::on_bgcolorSelect_clicked()
{
    QColorDialog *qcd = new QColorDialog(this);
    qcd->setAttribute(Qt::WA_DeleteOnClose);
    qcd->setCurrentColor(QColor(ui->bgcolor->text()));
    connect(qcd, &QColorDialog::colorSelected,
            this, [this](const QColor &color) {
        this->ui->bgcolor->setText(color.name());
        this->ui->bgcolor->setFocus();
        this->updateBgcolorWidgetSheet();
    });
    qcd->show();
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

void MainWindow::on_webSource_currentIndexChanged(int index)
{
    ui->webSourcePages->setCurrentIndex(index);
}
