#ifndef TABLES_MAXP_TABLE_HPP
#define TABLES_MAXP_TABLE_HPP

#include "otftable.hpp"

namespace fontutils
{

class MaxpTable : public OTFTable
{
public:
    Fixed version;
    size_t num_glyphs;
public:
    MaxpTable();
    virtual void parse(Buffer &dis) override;
    virtual Buffer compile() const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;
};

}

#endif
