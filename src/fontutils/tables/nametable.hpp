#ifndef TABLES_NAME_TABLE_HPP
#define TABLES_NAME_TABLE_HPP

#include "otftable.hpp"

#include <vector>

namespace fontutils
{

class NameTable : public OTFTable
{
public:
    uint16_t format;

    struct NameRecord {
        uint16_t platform_id;
        uint16_t encoding_id;
        uint16_t language_id;
        uint16_t name_id;
        std::string str;
    };
    std::vector<NameRecord> records;

public:
    NameTable();
    virtual void parse(Buffer &dis) override;
    virtual Buffer compile() const override;
};

}

#endif
