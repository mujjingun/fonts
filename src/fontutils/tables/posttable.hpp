#ifndef TABLE_POST_TABLE_HPP
#define TABLE_POST_TABLE_HPP

#include "otftable.hpp"

namespace fontutils
{

class PostTable : public OTFTable
{
public:
    Fixed version;
    Fixed italic_angle;
    int16_t underline_position;
    int16_t underline_thickness;
    uint32_t is_fixed_pitch;
    uint32_t min_mem_type_42;
    uint32_t max_mem_type_42;
    uint32_t min_mem_type_1;
    uint32_t max_mem_type_1;


public:
    PostTable();
    virtual void parse(Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
