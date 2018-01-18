#ifndef JAMOMODEL_HPP
#define JAMOMODEL_HPP

#include <QObject>
#include <QString>
#include <QList>
#include <QAbstractListModel>
#include <QPoint>

#include "fontutils/glyph.hpp"

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

    JamoModel(QObject *parent = nullptr);

    void addJamo(const Jamo &jamo);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Jamo> m_jamos;
};

#endif
