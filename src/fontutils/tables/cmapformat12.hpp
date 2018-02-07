#ifndef TABLES_CMAP_FORMAT_12_HPP
#define TABLES_CMAP_FORMAT_12_HPP

#include "cmapsubtable.hpp"

#include <map>

namespace fontutils
{

class CmapFormat12Subtable : public CmapSubtable
{
public:
    uint32_t language;

    // char -> gid mapping
    std::map<uint32_t, uint32_t> cmap;

public:
    CmapFormat12Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif