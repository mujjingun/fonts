#ifndef TABLES_CMAP_FORMAT_14_HPP
#define TABLES_CMAP_FORMAT_14_HPP

#include "cmapsubtable.hpp"

namespace fontutils
{

class CmapFormat14Subtable : public CmapSubtable
{
public:
    CmapFormat14Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
