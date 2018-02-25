#include "controller.hpp"

#include "fontutils/tables/cfftable.hpp"

#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

Controller::Controller(QObject* window, JamoModel* model)
    : window(window)
    , cons_model(model)
{
    connect(
        window,
        SIGNAL(fileImportSignal(QString)),
        this,
        SLOT(fileImportClicked(QString)));
    connect(
        window,
        SIGNAL(fileExportSignal(QString)),
        this,
        SLOT(fileExportClicked(QString)));

    connect(&load_watcher, SIGNAL(finished()), this, SLOT(fileLoadFinished()));
    connect(&load_watcher, SIGNAL(finished()), window, SIGNAL(fileLoadFinished()));

    connect(&save_watcher, SIGNAL(finished()), window, SIGNAL(fileExportFinished()));

    connect(
        this,
        SIGNAL(alert(int, QString, QString)),
        window,
        SIGNAL(alert(int, QString, QString)));
}

void Controller::fileImportClicked(QString file)
{
    QString filename = QDir::toNativeSeparators(QUrl(file).toLocalFile());

    load_future = QtConcurrent::run([=]() {
        geul::Font* ptr;
        try
        {
            ptr = new geul::Font;
            *ptr = geul::parse_otf(filename.toLocal8Bit().constData());
        }
        catch (std::exception const& e)
        {
            ptr = nullptr;
            emit alert(
                QMessageBox::Warning,
                "Error",
                QString("There was an error loading the font file: ")
                    + e.what());
        }
        return ptr;
    });
    load_watcher.setFuture(load_future);
}

void Controller::fileLoadFinished()
{
    auto ptr = std::unique_ptr<geul::Font>(load_future.result());
    if (!ptr)
        return;

    loaded_font = std::move(*ptr);
    cons_model->setGlyph(JamoName::KIYEOK, loaded_font.glyph(char32_t(0x3131)));

    qDebug() << "Font load finished.";
}

void Controller::fileExportClicked(QString file)
{
    QString filename = QDir::toNativeSeparators(QUrl(file).toLocalFile());

    save_future = QtConcurrent::run([=]() {
        try
        {
            geul::write_otf(loaded_font, filename.toLocal8Bit().constData());
        }
        catch (std::exception const& e)
        {
            emit alert(
                QMessageBox::Warning,
                "Error",
                QString("There was an error saving the font file: ")
                    + e.what());
            return false;
        }
        return true;
    });
    save_watcher.setFuture(save_future);
}
