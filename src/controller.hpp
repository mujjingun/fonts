#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QFutureWatcher>
#include <QMessageBox>

#include "jamomodel.hpp"
#include "fontutils/otfparser.hpp"

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *window, JamoModel *model);

public slots:
    void fileImportClicked(QString file);
    void fileLoadFinished();

    void fileExportClicked(QString file);

signals:
    void alert(int type, QString title, QString text);

private:
    QObject *window;
    JamoModel *cons_model;
    QFutureWatcher<void> load_watcher{}, save_watcher{};
    QFuture<geul::Font*> load_future;
    QFuture<bool> save_future;

    geul::Font loaded_font;
};

#endif
