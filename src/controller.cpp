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
    cons_model->setGlyph(JamoName::NIEUN, loaded_font.glyph(char32_t(0x3132)));
    cons_model->setGlyph(JamoName::TIKEUT, loaded_font.glyph(char32_t(0x3133)));
    cons_model->setGlyph(JamoName::RIEUL, loaded_font.glyph(char32_t(0x3134)));
    cons_model->setGlyph(JamoName::MIEUM, loaded_font.glyph(char32_t(0x3135)));
    cons_model->setGlyph(JamoName::PIEUP, loaded_font.glyph(char32_t(0x3136)));
    cons_model->setGlyph(JamoName::SIOS, loaded_font.glyph(char32_t(0x3137)));
    cons_model->setGlyph(JamoName::IEUNG, loaded_font.glyph(char32_t(0x3138)));
    cons_model->setGlyph(JamoName::CIEUC, loaded_font.glyph(char32_t(0x3139)));
    cons_model->setGlyph(JamoName::CHIEUCH, loaded_font.glyph(char32_t(0x3140)));
    cons_model->setGlyph(JamoName::KHIEUHK, loaded_font.glyph(char32_t(0x3141)));
    cons_model->setGlyph(JamoName::THIEUTH, loaded_font.glyph(char32_t(0x3142)));
    cons_model->setGlyph(JamoName::PHIEUPH, loaded_font.glyph(char32_t(0x3143)));
    cons_model->setGlyph(JamoName::HEIUH, loaded_font.glyph(char32_t(0x3144)));

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
