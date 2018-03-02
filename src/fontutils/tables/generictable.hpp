#ifndef TABLES_GENERIC_TABLE_HPP
#define TABLES_GENERIC_TABLE_HPP

#include "otftable.hpp"

namespace geul
{

class GenericTable : public OTFTable
{
    std::string data;

public:
    GenericTable(std::string tag, size_t length);
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;
};
}

#endif
