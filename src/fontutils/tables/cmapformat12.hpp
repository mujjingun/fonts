#ifndef TABLES_CMAP_FORMAT_12_HPP
#define TABLES_CMAP_FORMAT_12_HPP

#include "cmapsubtable.hpp"

#include <map>

namespace geul
{

class CmapFormat12Subtable : public CmapSubtable
{
public:
    uint32_t language = 0;

    // char -> gid mapping
    std::map<char32_t, uint32_t> cmap;

public:
    CmapFormat12Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;
};
}

#endif
