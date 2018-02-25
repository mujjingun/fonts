#ifndef JAMOMODEL_HPP
#define JAMOMODEL_HPP

#include <QObject>
#include <QString>
#include <QList>
#include <QAbstractListModel>
#include <QPoint>

#include <memory>
#include <map>

#include "fontutils/glyph.hpp"
#include "jamonames.hpp"
#include "formmodel.hpp"

class JamoModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum JamoModelRoles {
        NameRole = Qt::UserRole + 1,
        FormModelRole
    };

    JamoModel(QList<JamoName> names, QObject *parent = nullptr);

    void setGlyph(JamoName name, geul::Glyph const& glyph);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    std::map<JamoName, std::unique_ptr<FormModel>> m_jamos;
};

#endif
