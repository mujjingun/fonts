#include "basetable.hpp"

#include <cassert>
#include <functional>

namespace geul
{

BaseTable::BaseTable()
    : OTFTable(tag)
{}

namespace
{
BaseTable::Axis parse_axis(InputBuffer& dis)
{
    BaseTable::Axis axis;

    auto table_begin = dis.tell();

    std::streamoff base_tag_list_offset = dis.read<uint16_t>();
    std::streamoff base_script_list_offset = dis.read<uint16_t>();

    // BaseTagList table
    {
        auto lock = dis.seek_lock(table_begin + base_tag_list_offset);

        auto base_tag_count = dis.read<uint16_t>();

        axis.baseline_tags.resize(base_tag_count);

        for (auto& tags : axis.baseline_tags)
        {
            dis.read<uint8_t>(tags.data(), 4);
        }
    }

    // BaseScriptList table
    {
        auto lock = dis.seek_lock(table_begin + base_script_list_offset);
        auto table_begin = dis.tell();

        // baseScriptCount
        auto base_script_count = dis.read<uint16_t>();

        axis.script_records.resize(base_script_count);
        for (auto& script : axis.script_records)
        {
            // baseScriptTag
            dis.read<uint8_t>(script.tag.data(), 4);

            // baseScriptOffset
            // Offset to BaseScript table, from beginning of BaseScriptList
            std::streamoff script_offset = dis.read<uint16_t>();

            // BaseScript Table
            auto lock = dis.seek_lock(table_begin + script_offset);
            auto base_script_begin = dis.tell();

            // baseValuesOffset
            std::streamoff base_values_offset = dis.read<uint16_t>();
            if (base_values_offset)
            {
                auto lock
                    = dis.seek_lock(base_script_begin + base_values_offset);
                auto base_values_begin = dis.tell();

                // defaultBaselineIndex
                script.base_values.default_baseline_idx = dis.read<uint16_t>();

                // baseCoordCount
                auto base_coord_count = dis.read<uint16_t>();
                script.base_values.coords.resize(base_coord_count);
                for (auto& coord : script.base_values.coords)
                {
                    std::streamoff base_coord_off = dis.read<uint16_t>();
                    auto           lock
                        = dis.seek_lock(base_values_begin + base_coord_off);

                    auto format = dis.read<uint16_t>();
                    if (format == 1)
                    {
                        coord.coordinate = dis.read<int16_t>();
                    }
                    else
                    {
                        throw std::runtime_error(
                            "Unsupported BaseCoord format");
                    }
                }
            }
            else
            {
                throw std::runtime_error("baseValues is not present");
            }

            // defaultMinMaxOffset
            std::streamoff default_min_max_offset = dis.read<uint16_t>();
            if (default_min_max_offset)
            {
                throw std::runtime_error("defaultMinMaxOffset unimplemented");
            }

            // baseLangSysCount
            auto base_langsys_count = dis.read<uint16_t>();
            if (base_langsys_count)
            {
                throw std::runtime_error("baseLangSys unimplemented");
            }
        }
    }

    return axis;
}
}

void BaseTable::parse(InputBuffer& dis)
{
    auto beginning = dis.tell();

    // Major version of the BASE table = 1
    auto major = dis.read<uint16_t>();
    if (major != 1)
        throw std::runtime_error("Unrecognized BASE table major version");

    // Minor version of the BASE table
    auto version_minor = dis.read<uint16_t>();

    // Offset to horizontal Axis table,
    // from beginning of BASE table (may be NULL)
    std::streamoff horiz_axis_offset = dis.read<uint16_t>();

    // Offset to vertical Axis table,
    // from beginning of BASE table (may be NULL)
    std::streamoff vert_axis_offset = dis.read<uint16_t>();

    // Offset to Item Variation Store table,
    // from beginning of BASE table (may be null)
    int item_var_store_offset = 0;
    if (version_minor == 1)
    {
        item_var_store_offset = dis.read<uint32_t>();
    }

    if (horiz_axis_offset)
    {
        auto lock = dis.seek_lock(beginning + horiz_axis_offset);
        horiz_axis = parse_axis(dis);
    }

    if (vert_axis_offset)
    {
        auto lock = dis.seek_lock(beginning + vert_axis_offset);
        vert_axis = parse_axis(dis);
    }

    if (item_var_store_offset)
    {
        throw std::runtime_error("Item Variation Store table unimplemented");
    }
}

namespace
{
void write_axis(OutputBuffer& out, BaseTable::Axis const& axis)
{
    auto beginning = out.tell();

    auto base_tag_list_offset = out.tell();
    out.write<uint16_t>(0);

    auto base_script_list_offset = out.tell();
    out.write<uint16_t>(0);

    // BaseTagList table
    {
        out.write_at<uint16_t>(base_tag_list_offset, out.tell() - beginning);

        out.write<uint16_t>(axis.baseline_tags.size());

        for (auto const& tags : axis.baseline_tags)
        {
            out.write<uint8_t>(tags.data(), 4);
        }
    }

    // BaseScriptList table
    {
        auto list_begin = out.tell();

        out.write_at<uint16_t>(base_script_list_offset, out.tell() - beginning);

        out.write<uint16_t>(axis.script_records.size());

        std::vector<std::function<void(void)>> base_values_writers;
        for (auto const& script : axis.script_records)
        {
            out.write<uint8_t>(script.tag.data(), 4);

            // Placeholder for Offset to BaseScript table
            base_values_writers.push_back([&, pos = out.tell() ] {
                auto base_script_begin = out.tell();
                out.write_at<uint16_t>(pos, base_script_begin - list_begin);

                // BaseValuesOffset
                auto base_values_writer = [&, pos = out.tell() ]
                {
                    auto base_values_begin = out.tell();
                    out.write_at<uint16_t>(
                        pos, base_values_begin - base_script_begin);

                    out.write<uint16_t>(
                        script.base_values.default_baseline_idx);

                    out.write<uint16_t>(script.base_values.coords.size());

                    std::vector<std::function<void(void)>> base_coord_writers;
                    for (auto const& coord : script.base_values.coords)
                    {
                        base_coord_writers.push_back([&, pos = out.tell() ] {
                            out.write_at<uint16_t>(
                                pos, out.tell() - base_values_begin);

                            // coord format 1
                            out.write<uint16_t>(1);

                            out.write<int16_t>(coord.coordinate);
                        });
                        out.write<uint16_t>(0);
                    }

                    // write coords
                    for (auto const& f : base_coord_writers)
                        f();
                };
                out.write<uint16_t>(0);

                // defaultMinMaxOffset
                out.write<uint16_t>(0);

                // baseLangSysCount
                out.write<uint16_t>(0);

                base_values_writer();
            });
            out.write<uint16_t>(0);
        }

        // write basevalues
        for (auto const& f : base_values_writers)
            f();
    }
}
}

void BaseTable::compile(OutputBuffer& out) const
{
    auto beginning = out.tell();

    out.write<uint16_t>(1);
    out.write<uint16_t>(0);

    // Placeholder for Offset to horizontal Axis table
    auto hor_axis_off = out.tell();
    out.write<uint16_t>(0);

    // Placeholder for Offset to vertical Axis table
    auto vert_axis_off = out.tell();
    out.write<uint16_t>(0);

    out.write_at<uint16_t>(hor_axis_off, out.tell() - beginning);
    write_axis(out, horiz_axis);

    out.write_at<uint16_t>(vert_axis_off, out.tell() - beginning);
    write_axis(out, vert_axis);
}

bool BaseTable::operator==(const OTFTable& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<BaseTable const&>(rhs);

    return horiz_axis == other.horiz_axis && vert_axis == other.vert_axis;
}

bool BaseTable::Axis::operator==(const BaseTable::Axis& rhs) const noexcept
{
    return baseline_tags == rhs.baseline_tags
           && script_records == rhs.script_records;
}

bool BaseTable::Axis::BaseScriptRecord::
     operator==(const BaseTable::Axis::BaseScriptRecord& rhs) const noexcept
{
    return tag == rhs.tag && base_values == rhs.base_values;
}

bool BaseTable::Axis::BaseScriptRecord::BaseValues::
     operator==(const BaseTable::Axis::BaseScriptRecord::BaseValues& rhs) const
    noexcept
{
    return default_baseline_idx == rhs.default_baseline_idx
           && coords == rhs.coords;
}

bool BaseTable::Axis::BaseScriptRecord::BaseValues::BaseCoord::operator==(
    const BaseTable::Axis::BaseScriptRecord::BaseValues::BaseCoord& rhs) const
    noexcept
{
    return coordinate == rhs.coordinate;
}
}
