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
    CmapTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    uint32_t gid(char32_t utf32) const;

    static constexpr char const* tag = "cmap";

private:
    std::vector<std::unique_ptr<CmapSubtable>> subtables;

};
}

#endif
