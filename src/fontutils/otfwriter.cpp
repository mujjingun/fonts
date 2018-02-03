#include "makeotf/api/hotconv.h"
#include "parser.hpp"

#include <vector>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

namespace fontutils
{

struct FontWriter
{
    static void fatal(void *);
    static void message(void *, int type, char *text);
    static void *malloc(void *, size_t size);
    static void *realloc(void *, void *old, size_t size);
    static void free(void *, void *ptr);
    static char *psId(void *);
    static char *psRefill(void *, long *count);
    static char *cffId(void *);
    static void cffWrite1(void *, int c);
    static void cffWriteN(void *, long count, char *ptr);
    static char *cffSeek(void *, long offset, long *count);
    static char *cffRefill(void *, long *count);
    // static void cffSize(void *, long size, int euroAdded);
    static char *otfId(void *);
    static void otfWrite1(void *, int c);
    static void otfWriteN(void *, long count, char *ptr);
    static long otfTell(void *);
    static void otfSeek(void *, long offset);
    static char *otfRefill(void *, long *count);
    static char *featOpen(void *, char *name, long offset);
    static char *featRefill(void *, long *count);
    static void featClose(void *);
    static void featAddAnonData(void *, char *data, long count, unsigned long tag);
    static void tmpOpen(void *);
    static void tmpWriteN(void *, long count, char *ptr);
    static void tmpRewind(void *);
    static char *tmpRefill(void *, long *count);
    static void tmpClose(void *);
    static char *getFinalGlyphName(void *, char *gname);
    static char *getSrcGlyphName(void *, char *gname);
    static char *getUVOverrideName(void *, char *gname);
    // static void getAliasAndOrder(void *, char *oldName, char **newName, long int *order);
    static char *uvsOpen(void *, char *name);
    static char *uvsGetLine(void *,  char *buffer, long *count);
    static void uvsClose(void *);

    FontWriter(std::string const &filename);
    ~FontWriter();

    hotCtx ctx;
    std::string psdata, cffdata, otfdata, featuredata;
    std::ofstream out;
    std::ifstream feature_in;
};

FontWriter::FontWriter(std::string const &filename)
    : out(filename, std::ios::binary)
{
    hotCallbacks callbacks = {
        this,
        FontWriter::fatal,
        FontWriter::message,
        FontWriter::malloc,
        FontWriter::realloc,
        FontWriter::free,
        FontWriter::psId,
        FontWriter::psRefill,
        FontWriter::cffId,
        FontWriter::cffWrite1,
        FontWriter::cffWriteN,
        FontWriter::cffSeek,
        FontWriter::cffRefill,
        nullptr, // FontWriter::cffSize,
        FontWriter::otfId,
        FontWriter::otfWrite1,
        FontWriter::otfWriteN,
        FontWriter::otfTell,
        FontWriter::otfSeek,
        FontWriter::otfRefill,
        FontWriter::featOpen,
        FontWriter::featRefill,
        FontWriter::featClose,
        FontWriter::featAddAnonData,
        FontWriter::tmpOpen,
        FontWriter::tmpWriteN,
        FontWriter::tmpRewind,
        FontWriter::tmpRefill,
        FontWriter::tmpClose,
        FontWriter::getFinalGlyphName,
        FontWriter::getSrcGlyphName,
        FontWriter::getUVOverrideName,
        nullptr, //FontWriter::getAliasAndOrder,
        FontWriter::uvsOpen,
        FontWriter::uvsGetLine,
        FontWriter::uvsClose,
    };
    ctx = hotNew(&callbacks);
}

FontWriter::~FontWriter()
{
    hotFree(ctx);
}

void FontWriter::fatal(void *ctx)
{
    hotFree(static_cast<FontWriter*>(ctx)->ctx);
    throw std::runtime_error("Fatal error thrown by MakeOTF");
}

void FontWriter::message(void *, int type, char *text)
{
    if (type == hotNOTE) {
        std::cout << "MakeOTF note: " << text << std::endl;
    }
    else if (type == hotWARNING) {
        std::cout << "MakeOTF warning: " << text << std::endl;
    }
    else if (type == hotERROR) {
        std::cerr << "MakeOTF error: " << text << std::endl;
    }
    else if (type == hotFATAL) {
        std::cerr << "MakeOTF fatal error: " << text << std::endl;
    }
}

void *FontWriter::malloc(void *, size_t size)
{
    void *mem = std::malloc(size);
    if (!mem) throw std::bad_alloc();
    return mem;
}

void *FontWriter::realloc(void *, void *old, size_t size)
{
    void *mem = std::realloc(old, size);
    if (!mem) throw std::bad_alloc();
    return mem;
}

char *FontWriter::psId(void *)
{
    return const_cast<char*>("psId (Unimplemented)");
}

char *FontWriter::psRefill(void *ctx, long *count)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    *count = writer->psdata.size();
    return const_cast<char*>(writer->psdata.c_str());
}

char *FontWriter::cffId(void *)
{
    return const_cast<char*>("cffId (Unimplemented)");
}

void FontWriter::cffWrite1(void *ctx, int c)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    char byte = c;
    writer->out.write(&byte, 1);
}

void FontWriter::cffWriteN(void *ctx, long count, char *ptr)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    writer->out.write(ptr, count);
}

char *FontWriter::cffSeek(void *ctx, long offset, long *count)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);

    // out of bounds
    if (offset < 0 || size_t(offset) >= writer->cffdata.size())
    {
        *count = 0;
        return nullptr;
    }

    *count = writer->cffdata.size() - offset;
    return &writer->cffdata[0] + offset;
}

// Never called since all data is provided in cffSeek()
char *FontWriter::cffRefill(void *, long *count)
{
    *count = 0;
    return nullptr;
}

char *FontWriter::otfId(void *)
{
    return const_cast<char*>("otfId (Unimplemented)");
}

void FontWriter::otfWrite1(void *ctx, int c)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    char byte = c;
    writer->out.write(&byte, 1);
}

void FontWriter::otfWriteN(void *ctx, long count, char *ptr)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    writer->out.write(ptr, count);
}

long FontWriter::otfTell(void *ctx)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    return writer->out.tellp();
}

void FontWriter::otfSeek(void *ctx, long offset)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    writer->out.seekp(offset);
}

char *FontWriter::otfRefill(void *ctx, long *count)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    *count = writer->otfdata.size();
    return &writer->otfdata[0];
}

char *FontWriter::featOpen(void *ctx, char *name, long offset)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);

    if (!name) return nullptr;

    writer->feature_in = std::ifstream(name, std::ios::binary);
    writer->feature_in.seekg(offset);
    return name;
}

char *FontWriter::featRefill(void *ctx, long *count)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    *count = 8192;
    writer->featuredata.resize(*count);
    writer->feature_in.read(&writer->featuredata[0], *count);
    return &writer->featuredata[0];
}

void FontWriter::featClose(void *ctx)
{
    FontWriter *writer = static_cast<FontWriter*>(ctx);
    writer->featuredata.clear();
    writer->feature_in.close();
}

void FontWriter::featAddAnonData(void *, char *data, long count, unsigned long tag)
{
    throw std::runtime_error("unimplemented");
}

void FontWriter::tmpOpen(void *)
{
    throw std::runtime_error("unimplemented");
}

void FontWriter::tmpWriteN(void *, long count, char *ptr)
{
    throw std::runtime_error("unimplemented");
}

char *FontWriter::tmpRefill(void *, long *count)
{
    throw std::runtime_error("unimplemented");
}

void FontWriter::tmpClose(void *)
{
    throw std::runtime_error("unimplemented");
}

char *FontWriter::getFinalGlyphName(void *, char *gname)
{
    throw std::runtime_error("unimplemented");
}

char *FontWriter::getSrcGlyphName(void *, char *gname)
{
    throw std::runtime_error("unimplemented");
}

char *FontWriter::getUVOverrideName(void *, char *gname)
{
    throw std::runtime_error("unimplemented");
}

char *FontWriter::uvsOpen(void *, char *name)
{
    throw std::runtime_error("unimplemented");
}

char *FontWriter::uvsGetLine(void *, char *buffer, long *count)
{
    throw std::runtime_error("unimplemented");
}

void FontWriter::uvsClose(void *)
{
    throw std::runtime_error("unimplemented");
}

void writeOTF(Font const& font, std::string filename)
{
    FontWriter writer(filename);

    unsigned long hotConvertFlags = 0;
    hotConvertFlags |= HOT_ADD_STUB_DSIG;

    hotSetConvertFlags(writer.ctx, hotConvertFlags);
    //auto font_name = hotReadFont(ctx, )
    //hotAddName(ctx, );
    //hotAddMiscData(ctx, );
    //hotAddCMap(ctx, );
    hotConvert(writer.ctx);
}

}
