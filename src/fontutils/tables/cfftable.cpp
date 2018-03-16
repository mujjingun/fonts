#include "cfftable.hpp"

#include <cassert>
#include <sstream>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "../cffutils.hpp"
#include "../csparser.hpp"
#include "../stdstr.hpp"

namespace geul
{

CFFTable::CFFTable()
    : OTFTable(tag)
{}

void CFFTable::parse(InputBuffer& dis)
{
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
    dis.seek(beginning + std::streamoff(header_size));

    auto name_index = parse_index(dis);
    auto num_fonts = name_index.count;
    fonts.resize(num_fonts);

    for (auto name : name_index)
    {
        auto lock = dis.seek_lock(name.pos);
        fonts[name.index].name = dis.read_string(name.length);
    }

    // parse top dict index
    auto dict_index = parse_index(dis);

    // parse sid strings index
    auto                     string_index = parse_index(dis);
    std::vector<std::string> sid{ standard_strings.begin(),
                                  standard_strings.end() };
    for (auto str : string_index)
    {
        auto lock = dis.seek_lock(str.pos);
        sid.push_back(dis.read_string(str.length));
    }

    // indexviews for charstrings
    std::vector<IndexView> cs_indices;

    // local subroutines
    std::vector<std::vector<std::vector<std::string>>> lsubrs;

    // parse top dict
    for (auto dict : dict_index)
    {
        auto  lock = dis.seek_lock(dict.pos);
        auto& font = fonts[dict.index];
        auto& fontinfo = font.fontinfo;

        std::streampos charset_offset = -1;
        std::streampos charstrings_offset = -1;
        std::streampos fdarray_offset = -1;
        std::streampos fdselect_offset = -1;

        std::vector<CFFToken> operands;
        bool                  is_first_op = true;
        while (dis.tell() < dict.pos + std::streamoff(dict.length))
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
                    charset_offset = beginning + std::streampos(charset);
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
                charstrings_offset
                    = beginning + std::streamoff(operands[0].to_int());
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
                fdarray_offset
                    = beginning + std::streamoff(operands[0].to_int());
            }
            else if (op == CFFToken::Op::fdselect)
            {
                if (operands.size() != 1)
                    throw std::runtime_error(
                        "number of 'fdselect' operands != 1");
                fdselect_offset
                    = beginning + std::streamoff(operands[0].to_int());
            }
            else
            {
                throw std::runtime_error("Unknown operand in top dict.");
            }
            operands.clear();
        }

        // parse charstrings index
        if (charstrings_offset == -1)
            throw std::runtime_error(
                "charstrings offset not present in top dict.");
        dis.seek(charstrings_offset);
        auto cs_index = parse_index(dis);
        auto n_glyphs = cs_index.count;
        cs_indices.push_back(cs_index);

        // parse charset
        font.charset.resize(n_glyphs);
        font.charset[0] = 0;

        if (charset_offset == -1)
            throw std::runtime_error("charset offset not present in top dict.");
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
                int  n_left = dis.read<uint8_t>();
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

        if (fdselect_offset == -1)
            throw std::runtime_error(
                "fdselect offset not present in top dict.");
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
                int  first = dis.read<uint16_t>();
                auto fd = dis.read<uint8_t>();
                int  end = dis.peek<uint16_t>();
                for (int j = first; j < end; ++j)
                    font.fd_select[j] = fd;
            }
        }
        else
            throw std::runtime_error("Unrecognized fdselect format");

        // parse fdarray
        if (fdarray_offset == -1)
            throw std::runtime_error("fdarray offset not present in top dict.");
        dis.seek(fdarray_offset);
        auto fd_index = parse_index(dis);
        font.fd_array.resize(fd_index.count);
        lsubrs.emplace_back(fd_index.count);
        for (auto fditem : fd_index)
        {
            auto& font_dict = font.fd_array[fditem.index];
            int   priv_size, priv_offset = -1;

            dis.seek(fditem.pos);
            std::vector<CFFToken> operands;
            while (dis.tell() < fditem.pos + std::streamoff(fditem.length))
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
                    priv_offset
                        = beginning + std::streamoff(operands[1].to_int());
                }
                else
                {
                    throw std::runtime_error("Unknown operand in font dict.");
                }
                operands.clear();
            } // font dict

            // parse private dict
            if (priv_offset == -1)
                throw std::runtime_error("No private dict found");

            int subrs_offset = -1;

            operands.clear();

            dis.seek(priv_offset);
            while (int(dis.tell()) < priv_offset + priv_size)
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
                auto lock = dis.seek_lock(subrs_offset);
                auto subrs_index = parse_index(dis);
                for (auto item : subrs_index)
                {
                    auto lock = dis.seek_lock(item.pos);
                    lsubrs[dict.index][fditem.index].push_back(
                        dis.read_string(item.length));
                }
            }
        } // fdarray
    }

    // parse global subroutines
    auto                     gsubr_index = parse_index(dis);
    std::vector<std::string> gsubrs;
    for (auto item : gsubr_index)
    {
        auto lock = dis.seek_lock(item.pos);
        gsubrs.push_back(dis.read_string(item.length));
    }

    // parse charstrings
    for (auto i = 0u; i < fonts.size(); ++i)
    {
        auto& font = fonts[i];
        auto& index = cs_indices[i];
        font.glyphs.resize(index.count);
        for (auto item : index)
        {
            auto lock = dis.seek_lock(item.pos);
            int  fd_idx = font.fd_select[item.index];
            font.glyphs[item.index] = parse_charstring(
                dis.read_string(item.length),
                gsubrs,
                lsubrs[i][fd_idx],
                font.fd_array[fd_idx].default_width_x,
                font.fd_array[fd_idx].nominal_width_x);
        }
    }
}

namespace
{
std::unordered_map<std::string, int> make_sidmap(CFFTable const& cff)
{
    // string -> sid mapping
    std::unordered_map<std::string, int> sid_map;
    for (std::size_t i = 0u; i < standard_strings.size(); ++i)
        sid_map[standard_strings[i]] = i;

    int  sid_idx = standard_strings.size();
    auto add_sid = [&](std::string const& str) -> int {
        auto it = sid_map.find(str);
        if (it != sid_map.end())
            return it->second;
        else
        {
            sid_map.insert({ str, sid_idx });
            return sid_idx++;
        }
    };

    for (auto const& font : cff.fonts)
    {
        auto const& fontinfo = font.fontinfo;
        add_sid(fontinfo.registry);
        add_sid(fontinfo.ordering);
        add_sid(fontinfo.version);
        add_sid(fontinfo.notice);
        add_sid(fontinfo.copyright);
        add_sid(fontinfo.fullname);
        add_sid(fontinfo.familyname);
        add_sid(fontinfo.weight);
        add_sid(fontinfo.postscript);
        add_sid(fontinfo.basefont_name);

        for (auto const& fd : font.fd_array)
        {
            add_sid(fd.name);
        }
    }

    return sid_map;
}

struct TopDictOffsets
{
    std::vector<std::streampos> charset, charstr, fdarray, fdsel;
};

TopDictOffsets write_topdict(
    OutputBuffer&                               out,
    CFFTable const&                             cff,
    std::unordered_map<std::string, int> const& sid)
{
    TopDictOffsets offsets;

    write_index(out, cff.fonts.size(), [&](int idx) {
        auto const&                    font = cff.fonts[idx];
        auto const&                    fontinfo = font.fontinfo;
        const CFFTable::Font::FontInfo dflt{};

        // write ROS
        write_token(out, sid.at(fontinfo.registry));
        write_token(out, sid.at(fontinfo.ordering));
        write_token(out, fontinfo.supplement);
        write_token(out, CFFToken::Op::ros);

        // write normal top out entries
        if (dflt.version != fontinfo.version)
        {
            write_token(out, sid.at(fontinfo.version));
            write_token(out, CFFToken::Op::version);
        }

        if (dflt.notice != fontinfo.notice)
        {
            write_token(out, sid.at(fontinfo.notice));
            write_token(out, CFFToken::Op::notice);
        }

        if (dflt.copyright != fontinfo.copyright)
        {
            write_token(out, sid.at(fontinfo.copyright));
            write_token(out, CFFToken::Op::copyright);
        }

        if (dflt.fullname != fontinfo.fullname)
        {
            write_token(out, sid.at(fontinfo.fullname));
            write_token(out, CFFToken::Op::fullname);
        }

        if (dflt.familyname != fontinfo.familyname)
        {
            write_token(out, sid.at(fontinfo.familyname));
            write_token(out, CFFToken::Op::familyname);
        }

        if (dflt.weight != fontinfo.weight)
        {
            write_token(out, sid.at(fontinfo.weight));
            write_token(out, CFFToken::Op::weight);
        }

        if (dflt.is_fixed_pitch != fontinfo.is_fixed_pitch)
        {
            write_token(out, fontinfo.is_fixed_pitch);
            write_token(out, CFFToken::Op::isfixedpitch);
        }

        if (dflt.italic_angle != fontinfo.italic_angle)
        {
            write_token(out, fontinfo.italic_angle);
            write_token(out, CFFToken::Op::italicangle);
        }

        if (dflt.underline_position != fontinfo.underline_position)
        {
            write_token(out, fontinfo.underline_position);
            write_token(out, CFFToken::Op::underlineposition);
        }

        if (dflt.underline_thickness != fontinfo.underline_thickness)
        {
            write_token(out, fontinfo.underline_thickness);
            write_token(out, CFFToken::Op::underlinethickness);
        }

        if (dflt.paint_type != fontinfo.paint_type)
        {
            write_token(out, fontinfo.paint_type);
            write_token(out, CFFToken::Op::painttype);
        }

        if (dflt.charstring_type != fontinfo.charstring_type)
        {
            write_token(out, fontinfo.charstring_type);
            write_token(out, CFFToken::Op::charstringtype);
        }

        if (dflt.font_matrix != fontinfo.font_matrix)
        {
            for (double d : fontinfo.font_matrix)
                write_token(out, d);
            write_token(out, CFFToken::Op::fontmatrix);
        }

        if (dflt.unique_id != fontinfo.unique_id)
        {
            write_token(out, fontinfo.unique_id);
            write_token(out, CFFToken::Op::uniqueid);
        }

        if (dflt.font_bbox != fontinfo.font_bbox)
        {
            for (int i : fontinfo.font_bbox)
                write_token(out, i);
            write_token(out, CFFToken::Op::fontbbox);
        }

        if (dflt.stroke_width != fontinfo.stroke_width)
        {
            write_token(out, fontinfo.stroke_width);
            write_token(out, CFFToken::Op::strokewidth);
        }

        if (dflt.xuid != fontinfo.xuid)
        {
            for (int i : fontinfo.xuid)
                write_token(out, i);
            write_token(out, CFFToken::Op::xuid);
        }

        if (dflt.postscript != fontinfo.postscript)
        {
            write_token(out, sid.at(fontinfo.postscript));
            write_token(out, CFFToken::Op::postscript);
        }

        if (dflt.basefont_name != fontinfo.basefont_name)
        {
            write_token(out, sid.at(fontinfo.basefont_name));
            write_token(out, CFFToken::Op::basefontname);
        }

        if (dflt.basefont_blend != fontinfo.basefont_blend)
        {
            write_token(out, fontinfo.basefont_blend);
            write_token(out, CFFToken::Op::basefontblend);
        }

        // write CID font top out entries
        if (dflt.cid_font_version != fontinfo.cid_font_version)
        {
            write_token(out, fontinfo.cid_font_version);
            write_token(out, CFFToken::Op::cidfontversion);
        }

        if (dflt.cid_font_revision != fontinfo.cid_font_revision)
        {
            write_token(out, fontinfo.cid_font_revision);
            write_token(out, CFFToken::Op::cidfontrevision);
        }

        if (dflt.cid_font_type != fontinfo.cid_font_type)
        {
            write_token(out, fontinfo.cid_font_type);
            write_token(out, CFFToken::Op::cidfonttype);
        }

        if (dflt.cid_count != fontinfo.cid_count)
        {
            write_token(out, fontinfo.cid_count);
            write_token(out, CFFToken::Op::cidcount);
        }

        if (dflt.uid_base != fontinfo.uid_base)
        {
            write_token(out, fontinfo.uid_base);
            write_token(out, CFFToken::Op::uidbase);
        }

        // add placeholder for 'charset'
        {
            offsets.charset.push_back(out.tell());
            write_token(out, 0xfffffff);
            write_token(out, CFFToken::Op::charset);
        }

        // add placeholder for 'charstrings'
        {
            offsets.charstr.push_back(out.tell());
            write_token(out, 0xfffffff);
            write_token(out, CFFToken::Op::charstrings);
        }

        // add placeholder for 'fdselect'
        {
            offsets.fdsel.push_back(out.tell());
            write_token(out, 0xfffffff);
            write_token(out, CFFToken::Op::fdselect);
        }

        // add placeholder for 'fdarray'
        {
            offsets.fdarray.push_back(out.tell());
            write_token(out, 0xfffffff);
            write_token(out, CFFToken::Op::fdarray);
        }
    });

    return offsets;
}

void write_sidindex(
    OutputBuffer& out, std::unordered_map<std::string, int> const& sid_map)
{

    int                      std_size = standard_strings.size();
    std::vector<std::string> sids(sid_map.size() - std_size);
    for (auto const& item : sid_map)
    {
        if (item.second >= std_size)
            sids[item.second - std_size] = item.first;
    }
    write_index(
        out, sids.size(), [&](int idx) { out.write_string(sids[idx]); });
}

void write_charsets(
    OutputBuffer&                      out,
    CFFTable const&                    cff,
    std::streampos                     beginning,
    std::vector<std::streampos> const& offset_pos)
{
    int idx = 0;
    for (auto const& font : cff.fonts)
    {
        write_5byte_offset_at(out, offset_pos[idx], out.tell() - beginning);

        // Format 2
        out.write<uint8_t>(2);
        int start = font.charset[1], last = start;
        for (auto i = 2u; i < font.charset.size(); ++i)
        {
            int s = font.charset[i];
            if (last + 1 != s)
            {
                out.write<uint16_t>(start);
                out.write<uint16_t>(last - start);
                start = s;
            }
            last = s;
        }
        out.write<uint16_t>(start);
        out.write<uint16_t>(last - start);

        idx++;
    }
}

void write_fdsel(
    OutputBuffer&                      out,
    CFFTable const&                    cff,
    std::streampos                     beginning,
    std::vector<std::streampos> const& offset_pos)
{
    int idx = 0;
    for (auto const& font : cff.fonts)
    {
        write_5byte_offset_at(out, offset_pos[idx], out.tell() - beginning);

        // fdselect format 3
        out.write<uint8_t>(3);

        struct FDSelectRange
        {
            int first, fd;
        };
        std::vector<FDSelectRange> ranges;
        int                        start = 0, val = font.fd_select[0];
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
        ranges.push_back({ start, val });

        // number of ranges
        out.write<uint16_t>(ranges.size());

        for (auto range : ranges)
        {
            out.write<uint16_t>(range.first);
            out.write<uint8_t>(range.fd);
        }

        // sentinel GID
        out.write<uint16_t>(font.fd_select.size());

        idx++;
    }
}

void write_charstrs(
    OutputBuffer&                      out,
    CFFTable const&                    cff,
    std::streampos                     beginning,
    std::vector<std::streampos> const& offset_pos)
{
    int idx = 0;
    for (auto const& font : cff.fonts)
    {
        write_5byte_offset_at(out, offset_pos[idx], out.tell() - beginning);

        write_index(out, font.glyphs.size(), [&](int i) {
            write_charstring(out, font.glyphs[i]);
        });

        idx++;
    }
}

void write_lsubrs(
    OutputBuffer&                            out,
    CFFTable const&                          cff,
    std::vector<std::vector<std::streampos>> subr_offs,
    std::vector<std::vector<std::streampos>> priv_beginning)
{
    int idx = 0;
    for (auto const& font : cff.fonts)
    {
        int fd_idx = 0;
        for (auto const& font_dict : font.fd_array)
        {
            write_5byte_offset_at(
                out,
                subr_offs[idx][fd_idx],
                out.tell() - priv_beginning[idx][fd_idx]);

            // write empty subrs for now
            write_index(out, 0, nullptr);

            (void)font_dict;

            fd_idx++;
        }

        idx++;
    }
}

void write_privdict(
    OutputBuffer&                                   out,
    CFFTable const&                                 cff,
    std::streampos                                  beginning,
    std::vector<std::vector<std::streampos>> const& offset_pos)
{
    std::vector<std::vector<std::streampos>> subr_offs, priv_beginning;

    int idx = 0;
    for (auto const& font : cff.fonts)
    {
        priv_beginning.push_back({});
        subr_offs.push_back({});

        write_index(out, font.fd_array.size(), [&](int i) {
            auto offset = out.tell();
            priv_beginning.back().push_back(offset);

            auto const& font_dict = font.fd_array[i];

            const CFFTable::Font::FontDict dflt{};

            // write 'bluevalues'
            {
                int last = 0;
                for (int val : font_dict.blue_values)
                {
                    write_token(out, val - last);
                    last = val;
                }
                write_token(out, CFFToken::Op::bluevalues);
            }

            // write 'otherblues'
            {
                int last = 0;
                for (int val : font_dict.other_blues)
                {
                    write_token(out, val - last);
                    last = val;
                }
                write_token(out, CFFToken::Op::otherblues);
            }

            // write 'familyblues'
            {
                int last = 0;
                for (int val : font_dict.family_blues)
                {
                    write_token(out, val - last);
                    last = val;
                }
                write_token(out, CFFToken::Op::familyblues);
            }

            // write 'familyotherblues'
            {
                int last = 0;
                for (int val : font_dict.family_other_blues)
                {
                    write_token(out, val - last);
                    last = val;
                }
                write_token(out, CFFToken::Op::familyotherblues);
            }

            if (dflt.blue_scale != font_dict.blue_scale)
            {
                write_token(out, font_dict.blue_scale);
                write_token(out, CFFToken::Op::bluescale);
            }

            if (dflt.blue_shift != font_dict.blue_shift)
            {
                write_token(out, font_dict.blue_shift);
                write_token(out, CFFToken::Op::blueshift);
            }

            if (dflt.blue_fuzz != font_dict.blue_fuzz)
            {
                write_token(out, font_dict.blue_fuzz);
                write_token(out, CFFToken::Op::bluefuzz);
            }

            write_token(out, font_dict.std_hw);
            write_token(out, CFFToken::Op::stdhw);

            write_token(out, font_dict.std_vw);
            write_token(out, CFFToken::Op::stdvw);

            // write 'stemsnaph'
            {
                int last = 0;
                for (int val : font_dict.stem_snap_h)
                {
                    write_token(out, val - last);
                    last = val;
                }
                write_token(out, CFFToken::Op::stemsnaph);
            }

            // write 'stemsnapv'
            {
                int last = 0;
                for (int val : font_dict.stem_snap_v)
                {
                    write_token(out, val - last);
                    last = val;
                }
                write_token(out, CFFToken::Op::stemsnapv);
            }

            if (dflt.force_bold != font_dict.force_bold)
            {
                write_token(out, font_dict.force_bold);
                write_token(out, CFFToken::Op::forcebold);
            }

            if (dflt.language_group != font_dict.language_group)
            {
                write_token(out, font_dict.language_group);
                write_token(out, CFFToken::Op::languagegroup);
            }

            if (dflt.expansion_factor != font_dict.expansion_factor)
            {
                write_token(out, font_dict.expansion_factor);
                write_token(out, CFFToken::Op::expansionfactor);
            }

            if (dflt.initial_random_seed != font_dict.initial_random_seed)
            {
                write_token(out, font_dict.initial_random_seed);
                write_token(out, CFFToken::Op::initialrandomseed);
            }

            if (dflt.default_width_x != font_dict.default_width_x)
            {
                write_token(out, font_dict.default_width_x);
                write_token(out, CFFToken::Op::defaultwidthx);
            }

            if (dflt.nominal_width_x != font_dict.nominal_width_x)
            {
                write_token(out, font_dict.nominal_width_x);
                write_token(out, CFFToken::Op::nominalwidthx);
            }

            // 5-byte placeholder for local subr offset (from start of priv
            // dict)
            subr_offs.back().push_back(out.tell());
            write_token(out, 0xfffffff);
            write_token(out, CFFToken::Op::subrs);

            // write size and offset of this private dict in the fdarray
            auto len = out.tell() - offset;
            write_5byte_offset_at(out, offset_pos[idx][i], len);
            write_5byte_offset_at(
                out,
                offset_pos[idx][i] + std::streamoff(5),
                offset - beginning);
        });

        idx++;
    }

    // write Local Subr INDEX
    write_lsubrs(out, cff, subr_offs, priv_beginning);
}

void write_fdarray(
    OutputBuffer&                               out,
    CFFTable const&                             cff,
    std::streampos                              beginning,
    std::vector<std::streampos> const&          offset_pos,
    std::unordered_map<std::string, int> const& sid)
{
    std::vector<std::vector<std::streampos>> priv_offs;

    int idx = 0;
    for (auto const& font : cff.fonts)
    {
        write_5byte_offset_at(out, offset_pos[idx], out.tell() - beginning);

        priv_offs.push_back({});
        write_index(out, font.fd_array.size(), [&](int i) {
            write_token(out, sid.at(font.fd_array[i].name));
            write_token(out, CFFToken::Op::fontname);

            // placeholders for private dict size and offset
            priv_offs.back().push_back(out.tell());
            write_token(out, 0xfffffff);
            write_token(out, 0xfffffff);

            write_token(out, CFFToken::Op::private_);
        });

        idx++;
    }

    write_privdict(out, cff, beginning, priv_offs);
}
}

void CFFTable::compile(OutputBuffer& out) const
{
    auto beginning = out.tell();

    // major version
    out.write<uint8_t>(1);

    // minor version
    out.write<uint8_t>(0);

    // header size
    out.write<uint8_t>(4);

    // FIXME: offset size
    out.write<uint8_t>(4);

    // write name index
    write_index(
        out, fonts.size(), [&](int idx) { out.write_string(fonts[idx].name); });

    auto sid_map = make_sidmap(*this);

    // write Top Dict
    auto top_offsets = write_topdict(out, *this, sid_map);

    // write SID Strings INDEX
    write_sidindex(out, sid_map);

    // write gsubr INDEX
    // write empty index for now
    write_index(out, 0, nullptr);

    // write Charsets
    write_charsets(out, *this, beginning, top_offsets.charset);

    // write FDSelect (GID -> FD mapping)
    write_fdsel(out, *this, beginning, top_offsets.fdsel);

    // write CharStrings INDEX
    write_charstrs(out, *this, beginning, top_offsets.charstr);

    // write Font Dict Array
    write_fdarray(out, *this, beginning, top_offsets.fdarray, sid_map);
}

bool CFFTable::operator==(OTFTable const& rhs) const noexcept
{
    assert(typeid(*this) == typeid(rhs));
    auto const& other = static_cast<CFFTable const&>(rhs);

    return fonts == other.fonts;
}

bool CFFTable::Font::operator==(Font const& rhs) const noexcept
{
    return name == rhs.name && fontinfo == rhs.fontinfo
           && charset == rhs.charset && glyphs == rhs.glyphs
           && fd_select == rhs.fd_select && fd_array == rhs.fd_array;
}

bool CFFTable::Font::FontDict::operator==(FontDict const& rhs) const noexcept
{
    return name == rhs.name && blue_values == rhs.blue_values
           && other_blues == rhs.other_blues && family_blues == rhs.family_blues
           && family_other_blues == rhs.family_other_blues
           && blue_scale == rhs.blue_scale && blue_shift == rhs.blue_shift
           && blue_fuzz == rhs.blue_fuzz && std_hw == rhs.std_hw
           && std_vw == rhs.std_vw && stem_snap_h == rhs.stem_snap_h
           && stem_snap_v == rhs.stem_snap_v && force_bold == rhs.force_bold
           && language_group == rhs.language_group
           && expansion_factor == rhs.expansion_factor
           && initial_random_seed == rhs.initial_random_seed
           && default_width_x == rhs.default_width_x
           && nominal_width_x == rhs.nominal_width_x;
}

bool CFFTable::Font::FontInfo::operator==(FontInfo const& rhs) const noexcept
{
    return version == rhs.version && notice == rhs.notice
           && copyright == rhs.copyright && fullname == rhs.fullname
           && familyname == rhs.familyname && weight == rhs.weight
           && is_fixed_pitch == rhs.is_fixed_pitch
           && italic_angle == rhs.italic_angle
           && underline_position == rhs.underline_position
           && underline_thickness == rhs.underline_thickness
           && paint_type == rhs.paint_type
           && charstring_type == rhs.charstring_type
           && font_matrix == rhs.font_matrix && unique_id == rhs.unique_id
           && font_bbox == rhs.font_bbox && stroke_width == rhs.stroke_width
           && xuid == rhs.xuid && postscript == rhs.postscript
           && basefont_name == rhs.basefont_name
           && basefont_blend == rhs.basefont_blend && registry == rhs.registry
           && ordering == rhs.ordering && supplement == rhs.supplement
           && cid_font_version == rhs.cid_font_version
           && cid_font_revision == rhs.cid_font_revision
           && cid_font_type == rhs.cid_font_type && cid_count == rhs.cid_count
           && uid_base == rhs.uid_base;
}
}
