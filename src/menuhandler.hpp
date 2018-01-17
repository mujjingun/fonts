#ifndef MENUHANDLER_HPP
#define MENUHANDLER_HPP

#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QFutureWatcher>

#include "fontutils/parser.hpp"
#include "jamomodel.hpp"

class MenuHandler : public QObject
{
    Q_OBJECT
public:
    MenuHandler(QObject *window, JamoModel *model);

public slots:
    void fileOpenClicked(QVariant file);
    void loadFinished();

private:
    QObject *window;
    JamoModel *model;
    QFutureWatcher<void> watcher;
    QFuture<fontutils::Font> future;
};

#endif
