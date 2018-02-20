#include "cfftable.hpp"

#include <sstream>
#include <unordered_map>
#include <vector>

#include "../cffutils.hpp"
#include "../csparser.hpp"
#include "../stdstr.hpp"

namespace fontutils
{

CFFTable::CFFTable()
{
    id = "CFF ";
}

void CFFTable::parse(Buffer& dis)
{
    std::cout << "Parsing 'CFF '..." << std::endl;

    auto const beginning = dis.tell();

    auto major = dis.read<uint8_t>();
    if (major != 1)
        throw std::runtime_error("Unrecognized major CFF table version");

    // minor version (ignored)
    dis.read<uint8_t>();

    auto header_size = dis.read<uint8_t>();

    // offset size (FIXME: unused)
    dis.read<uint8_t>();

    // Seek to name index
    dis.seek(beginning + header_size);

    auto name_index = parse_index(dis);
    auto num_fonts = name_index.count;
    fonts.resize(num_fonts);

    for (auto name : name_index)
    {
        auto orig_pos = dis.seek(name.offset);
        fonts[name.index].name = dis.read_string(name.length);
        dis.seek(orig_pos);
    }

    // parse top dict index
    auto dict_index = parse_index(dis);

    // parse sid strings index
    auto string_index = parse_index(dis);
    std::vector<std::string> sid{ standard_strings.begin(),
                                  standard_strings.end() };
    for (auto str : string_index)
    {
        auto orig_pos = dis.seek(str.offset);
        sid.push_back(dis.read_string(str.length));
        dis.seek(orig_pos);
    }

    // indexviews for charstrings
    std::vector<IndexView> cs_indices;

    // local subroutines
    std::vector<std::vector<std::vector<std::string>>> lsubrs;

    // parse top dict
    for (auto dict : dict_index)
    {
        auto orig_pos = dis.seek(dict.offset);
        auto& font = fonts[dict.index];
        auto& fontinfo = font.fontinfo;

        int charset_offset = -1;
        size_t charstrings_offset;
        size_t fdarray_offset;
        size_t fdselect_offset;

        std::vector<CFFToken> operands;
        int is_first_op = true;
        while (dis.tell() < dict.offset + dict.length)
        {
            auto token = next_token(dis);
            if (token.get_type() & CFFToken::number)
            {
                operands.push_back(token);
                continue;
            }

            // parse operator
            CFFToken::Op op = token.get_op();

            // Should start with 'ros' operator for CID,
            //  'syntheticbase' for Synthetic fonts,
            //  and other for type 1.
            if (is_first_op)
            {
                if (op != CFFToken::Op::ros)
                    throw std::runtime_error("Font is not CID-keyed.");
                is_first_op = false;
            }

            if (op == CFFToken::Op::version)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'version' operands != 1");
                fontinfo.version = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::notice)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'notice' operands != 1");
                fontinfo.notice = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::copyright)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'copyright' operands != 1");
                fontinfo.copyright = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::fullname)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'fullname' operands != 1");
                fontinfo.fullname = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::familyname)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'familyname' operands != 1");
                fontinfo.familyname = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::weight)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'weight' operands != 1");
                fontinfo.weight = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::isfixedpitch)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'isfixedpitch' operands != 1");
                fontinfo.is_fixed_pitch = operands[0].to_int();
            }
            else if (op == CFFToken::Op::italicangle)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'italicangle' operands != 1");
                fontinfo.italic_angle = operands[0].to_int();
            }
            else if (op == CFFToken::Op::underlineposition)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'underlineposition' operands != 1");
                fontinfo.underline_position = operands[0].to_int();
            }
            else if (op == CFFToken::Op::underlinethickness)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'underlinethickness' operands != 1");
                fontinfo.underline_thickness = operands[0].to_int();
            }
            else if (op == CFFToken::Op::painttype)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'painttype' operands != 1");
                fontinfo.paint_type = operands[0].to_int();
            }
            else if (op == CFFToken::Op::charstringtype)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'isfixedpitch' operands != 1");
                fontinfo.charstring_type = operands[0].to_int();
            }
            else if (op == CFFToken::Op::fontmatrix)
            {
                if (operands.size() != 6)
                    throw std::runtime_error(
                        "number of 'fontmatrix' operands != 6");
                for (int i = 0; i < 6; ++i)
                    fontinfo.font_matrix[i] = operands[i].to_double();
            }
            else if (op == CFFToken::Op::uniqueid)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'uniqueid' operands != 1");
                fontinfo.unique_id = operands[0].to_int();
            }
            else if (op == CFFToken::Op::fontbbox)
            {
                if (operands.size() != 4)
                    throw std::runtime_error(
                        "number of 'fontbbox' operands != 4");
                for (int i = 0; i < 4; ++i)
                    fontinfo.font_bbox[i] = operands[i].to_int();
            }
            else if (op == CFFToken::Op::strokewidth)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'strokewidth' operands != 1");
                fontinfo.stroke_width = operands[0].to_int();
            }
            else if (op == CFFToken::Op::xuid)
            {
                for (auto& x : operands)
                    fontinfo.xuid.push_back(x.to_int());
            }
            else if (op == CFFToken::Op::charset)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'charset' operands != 1");
                auto charset = operands[0].to_int();
                if (charset > 2)
                    charset_offset = beginning + charset;
                else // TODO: implement standard charset
                    throw std::runtime_error("standard charset unimplemented");
            }
            else if (op == CFFToken::Op::encoding)
            {
                throw std::runtime_error("invalid operand 'encoding'");
            }
            else if (op == CFFToken::Op::charstrings)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'charstrings' operands != 1");
                charstrings_offset = beginning + operands[0].to_int();
            }
            else if (op == CFFToken::Op::syntheticbase)
            {
                throw std::runtime_error("invalid operand 'syntheticbase'");
            }
            else if (op == CFFToken::Op::postscript)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'postscript operands != 1");
                fontinfo.postscript = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::basefontname)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'basefontname' operands != 1");
                fontinfo.basefont_name = sid.at(operands[0].to_int());
            }
            else if (op == CFFToken::Op::basefontblend)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'basefontblend' operands != 1");
                fontinfo.basefont_blend = operands[0].to_int();
            }
            else if (op == CFFToken::Op::ros)
            {
                if (operands.size() != 3)
                    throw std::runtime_error("number of 'ros' operands != 3");
                fontinfo.registry = sid.at(operands[0].to_int());
                fontinfo.ordering = sid.at(operands[1].to_int());
                fontinfo.supplement = operands[2].to_int();
            }
            else if (op == CFFToken::Op::cidfontversion)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'cidfontversion' operands != 1");
                fontinfo.cid_font_version = operands[0].to_double();
            }
            else if (op == CFFToken::Op::cidfontrevision)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'cidfontrevision' operands != 1");
                fontinfo.cid_font_revision = operands[0].to_double();
            }
            else if (op == CFFToken::Op::cidfonttype)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'cidfonttype' operands != 1");
                fontinfo.cid_font_type = operands[0].to_int();
            }
            else if (op == CFFToken::Op::cidcount)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'cidcount' operands != 1");
                fontinfo.cid_count = operands[0].to_int();
            }
            else if (op == CFFToken::Op::uidbase)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'uidbase' operands != 1");
                fontinfo.uid_base = operands[0].to_int();
            }
            else if (op == CFFToken::Op::fdarray)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'fdarray' operands != 1");
                fdarray_offset = beginning + operands[0].to_int();
            }
            else if (op == CFFToken::Op::fdselect)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'fdselect' operands != 1");
                fdselect_offset = beginning + operands[0].to_int();
            }
            else
            {
                throw std::runtime_error("Unknown operand in top dict.");
            }
            operands.clear();
        }

        // parse charstrings index
        dis.seek(charstrings_offset);
        auto cs_index = parse_index(dis);
        auto n_glyphs = cs_index.count;
        cs_indices.push_back(cs_index);

        // parse charset
        font.charset.resize(n_glyphs);
        font.charset[0] = 0;

        dis.seek(charset_offset);
        auto charset_format = dis.read<uint8_t>();
        if (charset_format == 0)
        {
            for (int i = 1; i < n_glyphs; ++i)
                font.charset[i] = dis.read<uint16_t>();
        }
        else if (charset_format == 1)
        {
            for (int i = 1; i < n_glyphs;)
            {
                auto s = dis.read<uint16_t>();
                int n_left = dis.read<uint8_t>();
                while (i < n_glyphs && s <= s + n_left)
                    font.charset[i++] = s++;
            }
        }
        else if (charset_format == 2)
        {
            for (int i = 1; i < n_glyphs;)
            {
                int s = dis.read<uint16_t>();
                int n_left = dis.read<uint16_t>();
                int last = s + n_left;
                while (i < n_glyphs && s <= last)
                    font.charset[i++] = s++;
            }
        }
        else
            throw std::runtime_error("Unrecognized charset format");

        // parse fdselect
        font.fd_select.resize(n_glyphs);

        dis.seek(fdselect_offset);
        auto fdselect_format = dis.read<uint8_t>();
        if (fdselect_format == 0)
        {
            for (int i = 0; i < n_glyphs; ++i)
                font.fd_select[i] = dis.read<uint8_t>();
        }
        else if (fdselect_format == 3)
        {
            int n_ranges = dis.read<uint16_t>();
            for (int i = 0; i < n_ranges; ++i)
            {
                int first = dis.read<uint16_t>();
                auto fd = dis.read<uint8_t>();
                int end = dis.peek<uint16_t>();
                for (int j = first; j < end; ++j)
                    font.fd_select[j] = fd;
            }
        }
        else
            throw std::runtime_error("Unrecognized fdselect format");

        // parse fdarray
        dis.seek(fdarray_offset);
        auto fd_index = parse_index(dis);
        font.fd_array.resize(fd_index.count);
        lsubrs.emplace_back(fd_index.count);
        for (auto fditem : fd_index)
        {
            auto& font_dict = font.fd_array[fditem.index];
            size_t priv_size, priv_offset;

            dis.seek(fditem.offset);
            std::vector<CFFToken> operands;
            while (dis.tell() < fditem.offset + fditem.length)
            {
                auto token = next_token(dis);
                if (token.get_type() & CFFToken::number)
                {
                    operands.push_back(token);
                    continue;
                }

                // parse operator
                CFFToken::Op op = token.get_op();
                if (op == CFFToken::Op::fontname)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'fontname' operands != 1");
                    font_dict.name = sid.at(operands[0].to_int());
                }
                else if (op == CFFToken::Op::private_)
                {
                    priv_size = operands[0].to_int();
                    priv_offset = beginning + operands[1].to_int();
                }
                else
                {
                    throw std::runtime_error("Unknown operand in font dict.");
                }
                operands.clear();
            } // font dict

            // parse private dict
            int subrs_offset = -1;

            operands.clear();
            dis.seek(priv_offset);
            while (dis.tell() < priv_offset + priv_size)
            {
                auto token = next_token(dis);
                if (token.get_type() & CFFToken::number)
                {
                    operands.push_back(token);
                    continue;
                }

                // parse operator
                CFFToken::Op op = token.get_op();
                if (op == CFFToken::Op::bluevalues)
                {
                    font_dict.blue_values.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        font_dict.blue_values.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::otherblues)
                {
                    font_dict.other_blues.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        font_dict.other_blues.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::familyblues)
                {
                    font_dict.family_blues.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        font_dict.family_blues.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::familyotherblues)
                {
                    font_dict.family_other_blues.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        font_dict.family_other_blues.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::bluescale)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'bluescale' operands != 1");
                    font_dict.blue_scale = operands[0].to_double();
                }
                else if (op == CFFToken::Op::blueshift)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'blueshift' operands != 1");
                    font_dict.blue_shift = operands[0].to_double();
                }
                else if (op == CFFToken::Op::bluefuzz)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'bluefuzz' operands != 1");
                    font_dict.blue_fuzz = operands[0].to_double();
                }
                else if (op == CFFToken::Op::stdhw)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'stdhw' operands != 1");
                    font_dict.std_hw = operands[0].to_int();
                }
                else if (op == CFFToken::Op::stdvw)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'stdvw' operands != 1");
                    font_dict.std_vw = operands[0].to_int();
                }
                else if (op == CFFToken::Op::stemsnaph)
                {
                    font_dict.stem_snap_h.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        font_dict.stem_snap_h.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::stemsnapv)
                {
                    font_dict.stem_snap_v.clear();
                    int val = 0;
                    for (auto delta : operands)
                    {
                        val += delta.to_int();
                        font_dict.stem_snap_v.push_back(val);
                    }
                }
                else if (op == CFFToken::Op::forcebold)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'forcebold' operands != 1");
                    font_dict.force_bold = operands[0].to_int();
                }
                else if (op == CFFToken::Op::languagegroup)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'languagegroup' operands != 1");
                    font_dict.language_group = operands[0].to_int();
                }
                else if (op == CFFToken::Op::expansionfactor)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'expansionfactor' operands != 1");
                    font_dict.expansion_factor = operands[0].to_double();
                }
                else if (op == CFFToken::Op::initialrandomseed)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'initialrandomseed' operands != 1");
                    font_dict.initial_random_seed = operands[0].to_int();
                }
                else if (op == CFFToken::Op::subrs)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'subrs' operands != 1");
                    subrs_offset = priv_offset + operands[0].to_int();
                }
                else if (op == CFFToken::Op::defaultwidthx)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'defaultwidthx' operands != 1");
                    font_dict.default_width_x = operands[0].to_int();
                }
                else if (op == CFFToken::Op::nominalwidthx)
                {
                    if (operands.size() != 1)
                        throw std::runtime_error(
                            "number of 'nominalwidthx' operands != 1");
                    font_dict.nominal_width_x = operands[0].to_int();
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown operand in private dict.");
                }
                operands.clear();
            } // private dict

            // parse local subroutines
            if (subrs_offset != -1)
            {
                dis.seek(subrs_offset);
                auto subrs_index = parse_index(dis);
                for (auto item : subrs_index)
                {
                    dis.seek(item.offset);
                    lsubrs[dict.index][fditem.index].push_back(
                        dis.read_string(item.length));
                }
            }
        } // fdarray

        dis.seek(orig_pos);
    }

    // parse global subroutines
    auto gsubr_index = parse_index(dis);
    std::vector<std::string> gsubrs;
    for (auto item : gsubr_index)
    {
        auto orig_pos = dis.seek(item.offset);
        gsubrs.push_back(dis.read_string(item.length));
        dis.seek(orig_pos);
    }

    // parse charstrings
    for (auto i = 0u; i < fonts.size(); ++i)
    {
        auto& font = fonts[i];
        auto& index = cs_indices[i];
        font.glyphs.resize(index.count);
        for (auto item : index)
        {
            dis.seek(item.offset);
            font.glyphs[item.index] = parse_charstring(
                dis.read_string(item.length),
                gsubrs,
                lsubrs[i][font.fd_select[item.index]]);
        }
    }
}

Buffer CFFTable::compile() const
{
    Buffer buf;

    // major version
    buf.add<uint8_t>(1);

    // minor version
    buf.add<uint8_t>(0);

    // header size
    buf.add<uint8_t>(4);

    // FIXME: offset size
    buf.add<uint8_t>(4);

    // write name index
    {
        std::vector<Buffer> names;
        for (auto const& font : fonts)
            names.push_back(Buffer(font.name));
        buf.append(write_index(names));
    }

    // string -> sid mapping
    std::unordered_map<std::string, size_t> sid_map;
    // insert all standard strings
    for (size_t i = 0u; i < standard_strings.size(); ++i)
        sid_map[standard_strings[i]] = i;

    size_t sid_idx = standard_strings.size();
    auto get_sid = [&](std::string const& str) -> int {
        auto it = sid_map.find(str);
        if (it != sid_map.end())
            return it->second;
        else
        {
            sid_map.insert({ str, sid_idx });
            return sid_idx++;
        }
    };

    // buffer to store data to be added at the end
    Buffer append_buf;

    // write Charsets
    for (auto const& font : fonts)
    {
        std::ostringstream oss;
        oss << "charset/" << font.name;
        append_buf.add_marker(append_buf.size(), oss.str());

        // Format 2
        append_buf.add<uint8_t>(2);
        int start = font.charset[1], last = start;
        for (auto i = 2u; i < font.charset.size(); ++i)
        {
            int s = font.charset[i];
            if (last + 1 != s)
            {
                append_buf.add<uint16_t>(start);
                append_buf.add<uint16_t>(last - start);
                start = s;
            }
            last = s;
        }
        append_buf.add<uint16_t>(start);
        append_buf.add<uint16_t>(last - start);
    }

    // write FDSelect (GID -> FD mapping)
    for (auto const& font : fonts)
    {
        std::ostringstream oss;
        oss << "fdselect/" << font.name;
        append_buf.add_marker(append_buf.size(), oss.str());

        // fdselect format 3
        append_buf.add<uint8_t>(3);

        struct FDSelectRange
        {
            int first, fd;
        };
        std::vector<FDSelectRange> ranges;
        int start = 0, val = font.fd_select[0];
        for (auto i = 1u; i < font.fd_select.size(); ++i)
        {
            int fd = font.fd_select[i];
            if (val != fd)
            {
                ranges.push_back({ start, val });
                val = fd;
                start = i;
            }
        }
        // number of ranges
        append_buf.add<uint16_t>(ranges.size());

        for (auto range : ranges)
        {
            append_buf.add<uint16_t>(range.first);
            append_buf.add<uint8_t>(range.fd);
        }

        // sentinel GID
        append_buf.add<uint16_t>(font.fd_select.size());
    }

    // write CharStrings INDEX
    for (auto const& font : fonts)
    {
        std::ostringstream oss;
        oss << "charstrings/" << font.name;
        append_buf.add_marker(append_buf.size(), oss.str());

        std::vector<Buffer> index;
        for (auto const& glyph : font.glyphs)
        {
            index.push_back(write_charstring(glyph));
        }
        append_buf.append(write_index(index));
    }

    // write Font Dict Array
    for (auto const& font : fonts)
    {
        std::ostringstream oss;
        oss << "fdarray/" << font.name;
        append_buf.add_marker(append_buf.size(), oss.str());

        std::vector<Buffer> index;

        for (auto const& font_dict : font.fd_array)
        {
            Buffer fdbuf;
            write_token(fdbuf, get_sid(font_dict.name));
            write_token(fdbuf, CFFToken::Op::fontname);

            // write private dict size and offset
            std::ostringstream oss;
            oss << "private_off_size/" << font.name << "/" << font_dict.name;
            fdbuf.add_marker(fdbuf.size(), oss.str());
            write_token(fdbuf, 0xfffffff);
            write_token(fdbuf, 0xfffffff);
            write_token(fdbuf, CFFToken::Op::private_);

            index.push_back(std::move(fdbuf));
        }
        append_buf.append(write_index(index));
    }

    // write Private dict
    for (auto const& font : fonts)
    {
        std::vector<Buffer> index;
        for (auto const& font_dict : font.fd_array)
        {
            const Font::FontDict dflt{};
            Buffer pbuf;

            std::ostringstream start_marker_name;
            start_marker_name << "private/" << font.name << "/"
                              << font_dict.name;
            pbuf.add_marker(0, start_marker_name.str());

            // write 'bluevalues'
            {
                int last = 0;
                for (int val : font_dict.blue_values)
                {
                    write_token(pbuf, val - last);
                    last = val;
                }
                write_token(pbuf, CFFToken::Op::bluevalues);
            }

            // write 'otherblues'
            {
                int last = 0;
                for (int val : font_dict.other_blues)
                {
                    write_token(pbuf, val - last);
                    last = val;
                }
                write_token(pbuf, CFFToken::Op::otherblues);
            }

            // write 'familyblues'
            {
                int last = 0;
                for (int val : font_dict.family_blues)
                {
                    write_token(pbuf, val - last);
                    last = val;
                }
                write_token(pbuf, CFFToken::Op::familyblues);
            }

            // write 'familyotherblues'
            {
                int last = 0;
                for (int val : font_dict.family_other_blues)
                {
                    write_token(pbuf, val - last);
                    last = val;
                }
                write_token(pbuf, CFFToken::Op::familyotherblues);
            }

            if (dflt.blue_scale != font_dict.blue_scale)
            {
                write_token(pbuf, font_dict.blue_scale);
                write_token(pbuf, CFFToken::Op::bluescale);
            }

            if (dflt.blue_shift != font_dict.blue_shift)
            {
                write_token(pbuf, font_dict.blue_shift);
                write_token(pbuf, CFFToken::Op::blueshift);
            }

            if (dflt.blue_fuzz != font_dict.blue_fuzz)
            {
                write_token(pbuf, font_dict.blue_fuzz);
                write_token(pbuf, CFFToken::Op::bluefuzz);
            }

            write_token(pbuf, font_dict.std_hw);
            write_token(pbuf, CFFToken::Op::stdhw);

            write_token(pbuf, font_dict.std_vw);
            write_token(pbuf, CFFToken::Op::stdvw);

            // write 'stemsnaph'
            {
                int last = 0;
                for (int val : font_dict.stem_snap_h)
                {
                    write_token(pbuf, val - last);
                    last = val;
                }
                write_token(pbuf, CFFToken::Op::stemsnaph);
            }

            // write 'stemsnapv'
            {
                int last = 0;
                for (int val : font_dict.stem_snap_v)
                {
                    write_token(pbuf, val - last);
                    last = val;
                }
                write_token(pbuf, CFFToken::Op::stemsnapv);
            }

            if (dflt.force_bold != font_dict.force_bold)
            {
                write_token(pbuf, font_dict.force_bold);
                write_token(pbuf, CFFToken::Op::forcebold);
            }

            if (dflt.language_group != font_dict.language_group)
            {
                write_token(pbuf, font_dict.language_group);
                write_token(pbuf, CFFToken::Op::languagegroup);
            }

            if (dflt.expansion_factor != font_dict.expansion_factor)
            {
                write_token(pbuf, font_dict.expansion_factor);
                write_token(pbuf, CFFToken::Op::expansionfactor);
            }

            if (dflt.initial_random_seed != font_dict.initial_random_seed)
            {
                write_token(pbuf, font_dict.initial_random_seed);
                write_token(pbuf, CFFToken::Op::initialrandomseed);
            }

            if (dflt.default_width_x != font_dict.default_width_x)
            {
                write_token(pbuf, font_dict.default_width_x);
                write_token(pbuf, CFFToken::Op::defaultwidthx);
            }

            if (dflt.nominal_width_x != font_dict.nominal_width_x)
            {
                write_token(pbuf, font_dict.nominal_width_x);
                write_token(pbuf, CFFToken::Op::nominalwidthx);
            }

            // 5-byte placeholder for local subr offset (from start of priv
            // dict)
            std::ostringstream oss;
            oss << "local_subr_off/" << font.name << "/" << font_dict.name;
            pbuf.add_marker(pbuf.size(), oss.str());
            write_token(pbuf, 0xfffffff);
            write_token(pbuf, CFFToken::Op::subrs);

            std::ostringstream end_marker_name;
            end_marker_name << "private_end/" << font.name << "/"
                            << font_dict.name;
            pbuf.add_marker(pbuf.size(), end_marker_name.str());

            index.push_back(std::move(pbuf));
        }
        append_buf.append(write_index(index));
    }

    // write Local Subr INDEX
    for (auto const& font : fonts)
    {
        for (auto const& font_dict : font.fd_array)
        {
            std::ostringstream oss;
            oss << "local_subr/" << font.name << "/" << font_dict.name;
            append_buf.add_marker(append_buf.size(), oss.str());

            std::vector<Buffer> index;

            // write empty subrs for now
            append_buf.append(write_index(index));
        }
    }

    // write top dict
    std::vector<Buffer> dicts;
    for (auto const& font : fonts)
    {
        auto const& fontinfo = font.fontinfo;
        const Font::FontInfo dflt{};
        Buffer dict;

        // write ROS
        write_token(dict, get_sid(fontinfo.registry));
        write_token(dict, get_sid(fontinfo.ordering));
        write_token(dict, fontinfo.supplement);
        write_token(dict, CFFToken::Op::ros);

        // write normal top dict entries
        if (dflt.version != fontinfo.version)
        {
            write_token(dict, get_sid(fontinfo.version));
            write_token(dict, CFFToken::Op::version);
        }

        if (dflt.notice != fontinfo.notice)
        {
            write_token(dict, get_sid(fontinfo.notice));
            write_token(dict, CFFToken::Op::notice);
        }

        if (dflt.copyright != fontinfo.copyright)
        {
            write_token(dict, get_sid(fontinfo.copyright));
            write_token(dict, CFFToken::Op::copyright);
        }

        if (dflt.fullname != fontinfo.fullname)
        {
            write_token(dict, get_sid(fontinfo.fullname));
            write_token(dict, CFFToken::Op::fullname);
        }

        if (dflt.familyname != fontinfo.familyname)
        {
            write_token(dict, get_sid(fontinfo.familyname));
            write_token(dict, CFFToken::Op::familyname);
        }

        if (dflt.weight != fontinfo.weight)
        {
            write_token(dict, get_sid(fontinfo.weight));
            write_token(dict, CFFToken::Op::weight);
        }

        if (dflt.is_fixed_pitch != fontinfo.is_fixed_pitch)
        {
            write_token(dict, fontinfo.is_fixed_pitch);
            write_token(dict, CFFToken::Op::isfixedpitch);
        }

        if (dflt.italic_angle != fontinfo.italic_angle)
        {
            write_token(dict, fontinfo.italic_angle);
            write_token(dict, CFFToken::Op::italicangle);
        }

        if (dflt.underline_position != fontinfo.underline_position)
        {
            write_token(dict, fontinfo.underline_position);
            write_token(dict, CFFToken::Op::underlineposition);
        }

        if (dflt.underline_thickness != fontinfo.underline_thickness)
        {
            write_token(dict, fontinfo.underline_thickness);
            write_token(dict, CFFToken::Op::underlinethickness);
        }

        if (dflt.paint_type != fontinfo.paint_type)
        {
            write_token(dict, fontinfo.paint_type);
            write_token(dict, CFFToken::Op::painttype);
        }

        if (dflt.charstring_type != fontinfo.charstring_type)
        {
            write_token(dict, fontinfo.charstring_type);
            write_token(dict, CFFToken::Op::charstringtype);
        }

        if (dflt.font_matrix != fontinfo.font_matrix)
        {
            for (double d : fontinfo.font_matrix)
                write_token(dict, d);
            write_token(dict, CFFToken::Op::fontmatrix);
        }

        if (dflt.unique_id != fontinfo.unique_id)
        {
            write_token(dict, fontinfo.unique_id);
            write_token(dict, CFFToken::Op::uniqueid);
        }

        if (dflt.font_bbox != fontinfo.font_bbox)
        {
            for (int i : fontinfo.font_bbox)
                write_token(dict, i);
            write_token(dict, CFFToken::Op::fontbbox);
        }

        if (dflt.stroke_width != fontinfo.stroke_width)
        {
            write_token(dict, fontinfo.stroke_width);
            write_token(dict, CFFToken::Op::strokewidth);
        }

        if (dflt.xuid != fontinfo.xuid)
        {
            for (int i : fontinfo.xuid)
                write_token(dict, i);
            write_token(dict, CFFToken::Op::xuid);
        }

        if (dflt.postscript != fontinfo.postscript)
        {
            write_token(dict, get_sid(fontinfo.postscript));
            write_token(dict, CFFToken::Op::postscript);
        }

        if (dflt.basefont_name != fontinfo.basefont_name)
        {
            write_token(dict, get_sid(fontinfo.basefont_name));
            write_token(dict, CFFToken::Op::basefontname);
        }

        if (dflt.basefont_blend != fontinfo.basefont_blend)
        {
            write_token(dict, fontinfo.basefont_blend);
            write_token(dict, CFFToken::Op::basefontblend);
        }

        // write CID font top dict entries
        if (dflt.cid_font_version != fontinfo.cid_font_version)
        {
            write_token(dict, fontinfo.cid_font_version);
            write_token(dict, CFFToken::Op::cidfontversion);
        }

        if (dflt.cid_font_revision != fontinfo.cid_font_revision)
        {
            write_token(dict, fontinfo.cid_font_revision);
            write_token(dict, CFFToken::Op::cidfontrevision);
        }

        if (dflt.cid_font_type != fontinfo.cid_font_type)
        {
            write_token(dict, fontinfo.cid_font_type);
            write_token(dict, CFFToken::Op::cidfonttype);
        }

        if (dflt.cid_count != fontinfo.cid_count)
        {
            write_token(dict, fontinfo.cid_count);
            write_token(dict, CFFToken::Op::cidcount);
        }

        if (dflt.uid_base != fontinfo.uid_base)
        {
            write_token(dict, fontinfo.uid_base);
            write_token(dict, CFFToken::Op::uidbase);
        }

        // add placeholder for 'charset'
        {
            std::ostringstream oss;
            oss << "charset_off/" << font.name;
            dict.add_marker(dict.size(), oss.str());
            write_token(dict, 0xfffffff);
            write_token(dict, CFFToken::Op::charset);
        }

        // add placeholder for 'charstrings'
        {
            std::ostringstream oss;
            oss << "charstrings_off/" << font.name;
            dict.add_marker(dict.size(), oss.str());
            write_token(dict, 0xfffffff);
            write_token(dict, CFFToken::Op::charstrings);
        }

        // add placeholder for 'fdarray'
        {
            std::ostringstream oss;
            oss << "fdarray_off/" << font.name;
            dict.add_marker(dict.size(), oss.str());
            write_token(dict, 0xfffffff);
            write_token(dict, CFFToken::Op::fdarray);
        }

        // add placeholder for 'fdselect'
        {
            std::ostringstream oss;
            oss << "fdselect_off/" << font.name;
            dict.add_marker(dict.size(), oss.str());
            write_token(dict, 0xfffffff);
            write_token(dict, CFFToken::Op::fdselect);
        }

        dicts.push_back(std::move(dict));
    }
    buf.append(write_index(dicts));

    // write SID Strings INDEX
    {
        const size_t std_size = standard_strings.size();
        std::vector<Buffer> sids(sid_map.size() - std_size);
        for (auto const& item : sid_map)
        {
            if (item.second >= std_size)
                sids[item.second - std_size] = Buffer(item.first);
        }
        buf.append(write_index(sids));
    }

    // write gsubr INDEX
    {
        std::vector<Buffer> index;

        // write empty subrs for now
        buf.append(write_index(index));
    }

    // add rest of the data
    buf.append(append_buf);

    // write actual offset values to placeholders
    for (auto const& font : fonts)
    {
        // write charset offset values
        {
            std::ostringstream off;
            off << "charset_off/" << font.name;
            buf.seek(buf.marker(off.str()));

            std::ostringstream start;
            start << "charset/" << font.name;
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(buf.marker(start.str()));
        }

        // write charstrings offset values
        {
            std::ostringstream off;
            off << "charstrings_off/" << font.name;
            buf.seek(buf.marker(off.str()));

            std::ostringstream start;
            start << "charstrings/" << font.name;
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(buf.marker(start.str()));
        }

        // write fdarray offset values
        {
            std::ostringstream off;
            off << "fdarray_off/" << font.name;
            buf.seek(buf.marker(off.str()));

            std::ostringstream start;
            start << "fdarray/" << font.name;
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(buf.marker(start.str()));
        }

        // write fdselect offset values
        {
            std::ostringstream off;
            off << "fdselect_off/" << font.name;
            buf.seek(buf.marker(off.str()));

            std::ostringstream start;
            start << "fdselect/" << font.name;
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(buf.marker(start.str()));
        }

        for (auto const& font_dict : font.fd_array)
        {
            std::ostringstream priv_start_m;
            priv_start_m << "private/" << font.name << "/" << font_dict.name;
            size_t priv_start = buf.marker(priv_start_m.str());

            std::ostringstream priv_end_m;
            priv_end_m << "private_end/" << font.name << "/" << font_dict.name;
            size_t priv_end = buf.marker(priv_end_m.str());

            std::ostringstream subr_off;
            subr_off << "local_subr_off/" << font.name << "/" << font_dict.name;
            buf.seek(buf.marker(subr_off.str()));

            std::ostringstream subr_start_m;
            subr_start_m << "local_subr/" << font.name << "/" << font_dict.name;
            size_t subr_start = buf.marker(subr_start_m.str());

            // write 5-byte dict integer offset to local subr
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(subr_start - priv_start);

            std::ostringstream priv_off_size;
            priv_off_size << "private_off_size/" << font.name << "/"
                          << font_dict.name;
            buf.seek(buf.marker(priv_off_size.str()));

            // write size and offset of private dict
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(priv_end - priv_start);
            buf.write<uint8_t>(29);
            buf.write<uint32_t>(priv_start);
        }
    }

    // remove all offset markers
    buf.clear_markers();
    buf.seek(0);

    return buf;
}
}
