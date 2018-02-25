#include "cmapsubtable.hpp"

namespace geul
{

CmapSubtable::CmapSubtable(uint16_t platform_id, uint16_t encoding_id)
    : OTFTable("NO_ID")
    , platform_id(platform_id)
    , encoding_id(encoding_id)
{}
}
