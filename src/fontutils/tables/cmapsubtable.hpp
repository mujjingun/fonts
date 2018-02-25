#ifndef TABLES_CMAP_SUBTABLE_HPP
#define TABLES_CMAP_SUBTABLE_HPP

#include "otftable.hpp"

namespace geul
{

class CmapSubtable : public OTFTable
{
public:
    uint16_t platform_id;
    uint16_t encoding_id;
    CmapSubtable(uint16_t platform_id, uint16_t encoding_id);
};
}

#endif
