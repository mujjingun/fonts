#ifndef TABLES_CMAP_FORMAT_14_HPP
#define TABLES_CMAP_FORMAT_14_HPP

#include "cmapsubtable.hpp"
#include "unicodemap.hpp"

namespace fontutils
{

class CmapFormat14Subtable : public CmapSubtable
{
public:
    // variation selector -> (unicode -> gid[]) mapping
    // gid == -1 if default
    std::map<char32_t, UnicodeMap<int32_t>> uvsmap;

public:
    CmapFormat14Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
