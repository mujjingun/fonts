#ifndef TABLES_OFFSET_TABLE_HPP
#define TABLES_OFFSET_TABLE_HPP

#include "otftable.hpp"

#include "../glyph.hpp"

#include <map>
#include <memory>

namespace geul
{

class Font : public OTFTable
{
public:
    Font();
    virtual void   parse(Buffer& dis) override;
    virtual Buffer compile() const override;
    virtual bool   operator==(OTFTable const& rhs) const noexcept override;

    Glyph &glyph(char32_t ch);

private:
    std::map<std::string, std::unique_ptr<OTFTable>> tables;
};
}

#endif
