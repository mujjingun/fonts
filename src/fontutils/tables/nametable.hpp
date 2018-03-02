#ifndef TABLES_NAME_TABLE_HPP
#define TABLES_NAME_TABLE_HPP

#include "otftable.hpp"

#include <vector>

namespace geul
{

class NameTable : public OTFTable
{
public:
    uint16_t format;
    struct NameRecord
    {
        uint16_t    platform_id;
        uint16_t    encoding_id;
        uint16_t    language_id;
        uint16_t    name_id;
        std::string str;

        bool operator==(NameRecord const& rhs) const noexcept;
    };
    std::vector<NameRecord> records;

public:
    NameTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "name";
};
}

#endif
