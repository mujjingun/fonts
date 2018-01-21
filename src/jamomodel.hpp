#ifndef JAMOMODEL_HPP
#define JAMOMODEL_HPP

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QAbstractListModel>
#include <QPoint>

#include "fontutils/glyph.hpp"
#include "jamonames.hpp"

class Glyph : public QObject
{
    Q_OBJECT
public:
    Glyph(fontutils::Glyph g, QObject *parent);

    fontutils::Glyph glyph();

private:
    fontutils::Glyph m_glyph;
};
Q_DECLARE_METATYPE(Glyph*)

struct Jamo
{
    QString name;
    Glyph *glyph = nullptr;
};

class JamoModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum JamoModelRoles {
        NameRole = Qt::UserRole + 1,
        GlyphRole
    };

    JamoModel(QList<JamoName> names, QObject *parent = nullptr);

    void setJamo(JamoName name, Jamo jamo);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QMap<JamoName, Jamo> m_jamos;
};

#endif
