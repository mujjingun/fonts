#ifndef TABLE_POST_TABLE_HPP
#define TABLE_POST_TABLE_HPP

#include "otftable.hpp"

namespace geul
{

class PostTable : public OTFTable
{
public:
    Fixed    version;
    Fixed    italic_angle;
    int16_t  underline_position;
    int16_t  underline_thickness;
    uint32_t is_fixed_pitch;
    uint32_t min_mem_type_42;
    uint32_t max_mem_type_42;
    uint32_t min_mem_type_1;
    uint32_t max_mem_type_1;

public:
    PostTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "post";
};
}

#endif
