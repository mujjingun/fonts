#ifndef MENUHANDLER_HPP
#define MENUHANDLER_HPP

#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QFutureWatcher>

#include "jamomodel.hpp"

class MenuHandler : public QObject
{
    Q_OBJECT
public:
    MenuHandler(QObject *window, JamoModel *model);

public slots:
    void fileImportClicked(QVariant file);
    void loadFinished();

private:
    QObject *window;
    JamoModel *model;
    QFutureWatcher<void> watcher;
    //QFuture<fontutils::Font> future;
};

#endif
