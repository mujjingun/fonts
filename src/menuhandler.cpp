#include "menuhandler.hpp"

#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

MenuHandler::MenuHandler::MenuHandler(QObject *window, JamoModel *model)
    : window(window), model(model), watcher()
{ }

void MenuHandler::fileOpenClicked(QVariant file)
{
    QString filename = QDir::toNativeSeparators(QUrl(file.toString()).toLocalFile());

    connect(&watcher, SIGNAL(finished()), this, SLOT(loadFinished()));
    connect(&watcher, SIGNAL(finished()), window, SIGNAL(fileLoaded()));

    future = QtConcurrent::run([=]() {
        auto glyphs = fontutils::parse_font(filename.toLocal8Bit().constData());
        return glyphs;
    });
    watcher.setFuture(future);
}

void MenuHandler::loadFinished()
{
    fontutils::Font font = future.result();
    qDebug() << "Font load finished.";
    qDebug() << "Glyph count: " << font.glyphs.size();

    fontutils::Glyph g = font.glyphs[0];

    Jamo jamo{QString::fromStdString(g.chname)};

    model->addJamo(jamo);
}
