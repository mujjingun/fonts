#ifndef TABLES_CMAP_FORMAT_12_HPP
#define TABLES_CMAP_FORMAT_12_HPP

#include "cmapsubtable.hpp"

namespace fontutils
{

class CmapFormat12Subtable : public CmapSubtable
{
public:
    CmapFormat12Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
