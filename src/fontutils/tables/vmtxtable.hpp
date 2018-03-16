#ifndef TABLES_VMTXTABLE_HPP
#define TABLES_VMTXTABLE_HPP

#include "otftable.hpp"
#include <vector>

namespace geul {

class VmtxTable : public OTFTable
{
public:
    uint16_t advance_height;
    std::vector<int16_t> top_side_bearings;

public:
    VmtxTable(std::size_t num_glyphs, std::size_t num_v_metrics);
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "vmtx";
};

}

#endif // TABLES_VMTXTABLE_HPP
