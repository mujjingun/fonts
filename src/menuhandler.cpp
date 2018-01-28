#include "menuhandler.hpp"

#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

MenuHandler::MenuHandler::MenuHandler(QObject *window, JamoModel *model)
    : window(window), model(model), watcher()
{
    connect(window, SIGNAL(fileImportSignal(QVariant)),
                     this, SLOT(fileImportClicked(QVariant)));
    connect(&watcher, SIGNAL(finished()), this, SLOT(loadFinished()));
    connect(&watcher, SIGNAL(finished()), window, SIGNAL(fileLoaded()));
}

void MenuHandler::fileImportClicked(QVariant file)
{
    QString filename = QDir::toNativeSeparators(QUrl(file.toString()).toLocalFile());

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

    fontutils::Glyph g = font.glyphs.at(0x3131);
    model->setJamo(JamoName::KIYEOK, Jamo{QString::fromStdString(g.chname), new Glyph(g, model)});
    fontutils::Glyph g2 = font.glyphs.at(0x3147);
    model->setJamo(JamoName::IEUNG, Jamo{QString::fromStdString(g2.chname), new Glyph(g2, model)});
    fontutils::Glyph g3 = font.glyphs.at(0x314E);
    model->setJamo(JamoName::HEIUH, Jamo{QString::fromStdString(g3.chname), new Glyph(g3, model)});
}
