#ifndef FORMMODEL_H
#define FORMMODEL_H

#include <QAbstractListModel>

#include <memory>

#include "fontutils/glyph.hpp"

class GlyphObject : public QObject
{
    Q_OBJECT
public:
    GlyphObject(QObject* parent = 0);

    geul::Glyph glyph;
};
Q_DECLARE_METATYPE(GlyphObject*)

class FormModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(GlyphObject* defaultGlyph READ defaultGlyphObject WRITE
                   setDefaultGlyphObject NOTIFY defaultGlyphObjectChanged)

public:
    explicit FormModel(QObject* parent = 0);

    void setDefaultGlyphObject(GlyphObject* glyph);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    enum Roles
    {
        NameRole = Qt::UserRole + 1,
        GlyphRole,
    };

    QHash<int, QByteArray> roleNames() const override;

    // properties
    GlyphObject* defaultGlyphObject();
    void         setDefaultGlyph(geul::Glyph const& glyph);

signals:
    void defaultGlyphObjectChanged();

private:
    GlyphObject* m_defaultglyph;
};

#endif // FORMMODEL_H
