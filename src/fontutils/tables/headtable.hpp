#ifndef TABLES_HEAD_TABLE_HPP
#define TABLES_HEAD_TABLE_HPP

#include "otftable.hpp"
#include "../types.hpp"

#include <limits>

namespace geul
{

class HeadTable : public OTFTable
{
public:
    int16_t xmin = std::numeric_limits<int16_t>::min();
    int16_t ymin = std::numeric_limits<int16_t>::min();
    int16_t xmax = std::numeric_limits<int16_t>::max();
    int16_t ymax = std::numeric_limits<int16_t>::max();

    uint16_t units_per_em = 1000;

    uint16_t mac_style;
    uint16_t lowest_PPEM;
    int16_t  font_direction_hint;

    int16_t glyph_data_format;

    Fixed version;
    Fixed font_revision;

    uint16_t flags;

    uint64_t created;
    uint64_t modified;

    enum Flags
    {
        BASELINE_AT_ZERO = 1 << 0,
        LSB_AT_ZERO = 1 << 1,
        SIZE_SPECIFIC_INSTRUCTIONS = 1 << 2,
        FORCE_PPEM_INTEGER_VALUES = 1 << 3
    };

public:
    HeadTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "head";
};
}

#endif
