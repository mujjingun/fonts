#ifndef TABLES_HMTX_TABLE_HPP
#define TABLES_HMTX_TABLE_HPP

#include "otftable.hpp"

#include <vector>

namespace fontutils
{

class HmtxTable : public OTFTable
{
public:
    struct HMetric
    {
        uint16_t advance_width;
        int16_t lsb;
    };

    std::vector<HMetric> metrics;
    std::vector<int16_t> lsbs;

public:
    HmtxTable(size_t num_glyphs, size_t num_h_metrics);
    virtual void parse (Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
