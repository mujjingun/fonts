#ifndef TABLES_CMAP_FORMAT_4_HPP
#define TABLES_CMAP_FORMAT_4_HPP

#include "cmapsubtable.hpp"

#include <map>

namespace fontutils
{

class CmapFormat4Subtable : public CmapSubtable
{
public:
    uint16_t language;

    /// code point -> gid mapping, NOT cid
    std::map<uint16_t, uint16_t> cmap;

public:
    CmapFormat4Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
