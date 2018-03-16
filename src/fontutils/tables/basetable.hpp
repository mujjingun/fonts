#ifndef TABLE_BASETABLE_HPP
#define TABLE_BASETABLE_HPP

#include "otftable.hpp"
#include "../types.hpp"

#include <vector>

namespace geul
{

class BaseTable : public OTFTable
{
public:
    struct Axis
    {
        std::vector<Tag> baseline_tags;

        struct BaseScriptRecord
        {
            Tag tag;

            struct BaseValues
            {
                uint16_t default_baseline_idx;

                struct BaseCoord
                {
                    // only format 1 is supported
                    int16_t coordinate;

                    bool operator==(BaseCoord const& rhs) const noexcept;
                };
                std::vector<BaseCoord> coords;

                bool operator==(BaseValues const& rhs) const noexcept;
            } base_values;

            bool operator==(BaseScriptRecord const& rhs) const noexcept;
        };
        std::vector<BaseScriptRecord> script_records;

        bool operator==(Axis const& rhs) const noexcept;
    };

    Axis horiz_axis;
    Axis vert_axis;

public:
    BaseTable();
    virtual void parse(InputBuffer& dis) override;
    virtual void compile(OutputBuffer& out) const override;
    virtual bool operator==(OTFTable const& rhs) const noexcept override;

    static constexpr char const* tag = "BASE";
};
}

#endif
