#ifndef TABLES_CMAP_FORMAT_14_HPP
#define TABLES_CMAP_FORMAT_14_HPP

#include "cmapsubtable.hpp"

#include <map>
#include <vector>

namespace fontutils
{

class CmapFormat14Subtable : public CmapSubtable
{
public:
    struct DefaultUVSRange
    {
        char32_t start_val;
        int count;
    };
    using DefaultUVSTable = std::vector<DefaultUVSRange>;
    struct UVSMapping
    {
        char32_t unicode;
        uint16_t gid;
    };
    using UVSMappingTable = std::vector<UVSMapping>;
    struct UVS
    {
        DefaultUVSTable dflt;
        UVSMappingTable special;
    };
    std::map<char32_t, UVS> map;

public:
    CmapFormat14Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
