#ifndef TABLES_CMAP_TABLE_HPP
#define TABLES_CMAP_TABLE_HPP

#include "cmapsubtable.hpp"
#include "otftable.hpp"

#include <memory>
#include <vector>

namespace geul
{

class CmapTable : public OTFTable
{
public:
    std::vector<std::unique_ptr<CmapSubtable>> subtables;

public:
    CmapTable();
    virtual void   parse(Buffer& dis) override;
    virtual Buffer compile() const override;
    virtual bool   operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "cmap";
};
}

#endif
