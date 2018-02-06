#ifndef TABLES_CMAP_FORMAT_4_HPP
#define TABLES_CMAP_FORMAT_4_HPP

#include "cmapsubtable.hpp"

namespace fontutils
{

class CmapFormat4Subtable : public CmapSubtable
{
public:
    uint16_t length;
    uint16_t language;

public:
    CmapFormat4Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
