#ifndef TABLES_HMTX_TABLE_HPP
#define TABLES_HMTX_TABLE_HPP

#include "otftable.hpp"

#include <vector>

namespace geul
{

class HmtxTable : public OTFTable
{
public:
    struct HMetric
    {
        uint16_t advance_width;
        int16_t  lsb;

        bool operator==(HMetric const& rhs) const noexcept;
    };

    std::vector<HMetric> metrics;
    std::vector<int16_t> lsbs;

public:
    HmtxTable(std::size_t num_glyphs, std::size_t num_h_metrics);
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "hmtx";
};
}

#endif
