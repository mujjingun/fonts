#ifndef TABLES_GENERIC_TABLE_HPP
#define TABLES_GENERIC_TABLE_HPP

#include "otftable.hpp"

namespace fontutils
{

class GenericTable : public OTFTable
{
    std::string data;

public:
    GenericTable(std::string tag, size_t length);
    virtual void parse(Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
