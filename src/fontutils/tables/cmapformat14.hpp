#ifndef TABLES_CMAP_FORMAT_14_HPP
#define TABLES_CMAP_FORMAT_14_HPP

#include "cmapsubtable.hpp"

#include <map>
#include <vector>

namespace geul
{

class CmapFormat14Subtable : public CmapSubtable
{
public:
    struct DefaultUVSRange
    {
        char32_t start_val;
        int      count;

        bool operator==(DefaultUVSRange const& rhs) const noexcept;
    };
    using DefaultUVSTable = std::vector<DefaultUVSRange>;
    struct UVSMapping
    {
        char32_t unicode;
        uint16_t gid;

        bool operator==(UVSMapping const& rhs) const noexcept;
    };
    using UVSMappingTable = std::vector<UVSMapping>;
    struct UVS
    {
        DefaultUVSTable dflt;
        UVSMappingTable special;

        bool operator==(UVS const& rhs) const noexcept;
    };
    std::map<char32_t, UVS> map;

public:
    CmapFormat14Subtable(uint16_t platform_id, uint16_t encoding_id);
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;
};
}

#endif
