#ifndef TABLES_OFFSET_TABLE_HPP
#define TABLES_OFFSET_TABLE_HPP

#include "otftable.hpp"

#include <map>
#include <memory>

namespace fontutils
{

class OffsetTable : public OTFTable
{
public:
    std::map<std::string, std::unique_ptr<OTFTable>> tables;

public:
    OffsetTable();
    virtual void parse(Buffer &dis) override;
    virtual Buffer compile() const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;
};

}

#endif
