#ifndef TABLES_CMAP_TABLE_HPP
#define TABLES_CMAP_TABLE_HPP

#include "otftable.hpp"
#include "cmapsubtable.hpp"

#include <vector>
#include <memory>

namespace fontutils
{

class CmapTable : public OTFTable
{
public:
    std::vector<std::unique_ptr<CmapSubtable>> subtables;

public:
    CmapTable();
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;
};

}

#endif
