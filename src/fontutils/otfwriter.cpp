#include "makeotf/api/hotconv.h"
#include "parser.hpp"

#include <vector>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <functional>

namespace fontutils
{

template <typename F, F f> struct callback_helper;
template <typename Return, typename Object, typename... Args, Return (Object::*F)(Args...)>
struct callback_helper<Return (Object::*)(Args...), F>
{
    static Return callback(void *ctx, Args ... args)
    {
        Object *obj = static_cast<Object*>(ctx);
        return (obj->*F)(args...);
    }
};

template<typename F, F f>
auto make_callback = &callback_helper<F, f>::callback;
#define CB(F) make_callback<decltype(&F), &F>

struct FontWriter
{
    // Error handling
    void fatal();
    void message(int type, char *text);

    // Memory allocation
    void *malloc(size_t size);
    void *realloc(void *old, size_t size);
    void free(void *ptr);

    // Postscript input
    char *psId();
    char *psRefill(long *count);

    // CFF input
    char *cffId();
    char *cffRefill(long *count);
    char *cffSeek(long offset, long *count);

    // CFF output
    void cffWrite1(int c);
    void cffWriteN(long count, char *ptr);

    // OTF input
    char *otfId();
    char *otfRefill(long *count);

    // OTF output
    void otfWrite1(int c);
    void otfWriteN(long count, char *ptr);
    void otfSeek(long offset);
    long otfTell();

    // Feature IO
    char *featOpen(char *name, long offset);
    char *featRefill(long *count);
    void featClose();
    void featAddAnonData(char *data, long count, unsigned long tag);

    // Temp file IO
    void tmpOpen();
    void tmpWriteN(long count, char *ptr);
    void tmpRewind();
    char *tmpRefill(long *count);
    void tmpClose();

    // Feature file identifier conversion
    char *getFinalGlyphName(char *gname);
    char *getSrcGlyphName(char *gname);
    char *getUVOverrideName(char *gname);

    // Unicode variation selector
    char *uvsOpen(char *name);
    char *uvsGetLine(char *buffer, long *count);
    void uvsClose();

    // CMap input
    char *CMapId();
    char *CMapRefill(long *count);

    FontWriter(std::string const &filename, std::string const& psdata, std::string const& cmapdata);
    ~FontWriter();

    void convert();

    hotCtx ctx;
    std::ofstream out;
    std::string psdata, cffdata, otfdata, featuredata, cmapdata;
    std::ifstream feature, temp;
};

FontWriter::FontWriter(std::string const &filename, std::string const& psdata, std::string const& cmapdata)
    : out(filename, std::ios::binary), psdata(psdata), cmapdata(cmapdata)
{
    hotCallbacks callbacks;
    callbacks.ctx = this;
    callbacks.fatal             = CB(FontWriter::fatal);
    callbacks.message           = CB(FontWriter::message);
    callbacks.malloc            = CB(FontWriter::malloc);
    callbacks.realloc           = CB(FontWriter::realloc);
    callbacks.free              = CB(FontWriter::free);
    callbacks.psId              = CB(FontWriter::psId);
    callbacks.psRefill          = CB(FontWriter::psRefill);
    callbacks.cffId             = CB(FontWriter::cffId);
    callbacks.cffWrite1         = CB(FontWriter::cffWrite1);
    callbacks.cffWriteN         = CB(FontWriter::cffWriteN);
    callbacks.cffSeek           = CB(FontWriter::cffSeek);
    callbacks.cffRefill         = CB(FontWriter::cffRefill);
    callbacks.cffSize           = nullptr;
    callbacks.otfId             = CB(FontWriter::otfId);
    callbacks.otfWrite1         = CB(FontWriter::otfWrite1);
    callbacks.otfWriteN         = CB(FontWriter::otfWriteN);
    callbacks.otfTell           = CB(FontWriter::otfTell);
    callbacks.otfSeek           = CB(FontWriter::otfSeek);
    callbacks.otfRefill         = CB(FontWriter::otfRefill);
    callbacks.featOpen          = CB(FontWriter::featOpen);
    callbacks.featRefill        = CB(FontWriter::featRefill);
    callbacks.featClose         = CB(FontWriter::featClose);
    callbacks.featAddAnonData   = CB(FontWriter::featAddAnonData);
    callbacks.tmpOpen           = CB(FontWriter::tmpOpen);
    callbacks.tmpWriteN         = CB(FontWriter::tmpWriteN);
    callbacks.tmpRewind         = CB(FontWriter::tmpRewind);
    callbacks.tmpRefill         = CB(FontWriter::tmpRefill);
    callbacks.tmpClose          = CB(FontWriter::tmpClose);
    callbacks.getFinalGlyphName = CB(FontWriter::getFinalGlyphName);
    callbacks.getSrcGlyphName   = CB(FontWriter::getSrcGlyphName);
    callbacks.getUVOverrideName = CB(FontWriter::getUVOverrideName);
    callbacks.getAliasAndOrder  = nullptr;
    callbacks.uvsOpen           = CB(FontWriter::uvsOpen);
    callbacks.uvsGetLine        = CB(FontWriter::uvsGetLine);
    callbacks.uvsClose          = CB(FontWriter::uvsClose);

    ctx = hotNew(&callbacks);

    // Set flags before any hot functions are called
    unsigned long hotConvertFlags = 0;
    hotConvertFlags |= HOT_ADD_STUB_DSIG;
    hotSetConvertFlags(ctx, hotConvertFlags);

    // Read fontname
    hotReadFontOverrides fontOverrides;
    int psinfo;
    std::string fontname = hotReadFont(ctx, 0, &psinfo, &fontOverrides);

    hotAddName(ctx, 1, 0, 0, 0, const_cast<char*>("Copyright Info"));

    hotAddCMap(ctx, CB(FontWriter::CMapId), CB(FontWriter::CMapRefill));
}

FontWriter::~FontWriter()
{
    hotFree(ctx);
}

/* Callbacks */

void FontWriter::fatal()
{
    hotFree(ctx);
    throw std::runtime_error("Fatal error thrown by MakeOTF");
}

void FontWriter::message(int type, char *text)
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

void *FontWriter::malloc(size_t size)
{
    void *mem = std::malloc(size);
    if (!mem) throw std::bad_alloc();
    return mem;
}

void *FontWriter::realloc(void *old, size_t size)
{
    void *mem = std::realloc(old, size);
    if (!mem) throw std::bad_alloc();
    return mem;
}

void FontWriter::free(void *ptr)
{
    std::free(ptr);
}

char *FontWriter::psId()
{
    return const_cast<char*>("psId (Unimplemented)");
}

char *FontWriter::psRefill(long *count)
{
    *count = psdata.size();
    return const_cast<char*>(psdata.c_str());
}

char *FontWriter::cffId()
{
    return const_cast<char*>("cffId (Unimplemented)");
}

void FontWriter::cffWrite1(int c)
{
    char byte = c;
    out.write(&byte, 1);
}

void FontWriter::cffWriteN(long count, char *ptr)
{
    out.write(ptr, count);
}

char *FontWriter::cffSeek(long offset, long *count)
{
    // out of bounds
    if (offset < 0)
    {
        *count = 0;
        return nullptr;
    }

    *count = 150000;
    if (size_t(offset + *count) > cffdata.size())
    {
        cffdata.resize(offset + *count);
    }
    return &cffdata[0] + offset;
}

// Never called since all data is provided in cffSeek()
char *FontWriter::cffRefill(long *count)
{
    *count = 0;
    return nullptr;
}

char *FontWriter::otfId()
{
    return const_cast<char*>("otfId (Unimplemented)");
}

void FontWriter::otfWrite1(int c)
{
    char byte = c;
    out.write(&byte, 1);
}

void FontWriter::otfWriteN(long count, char *ptr)
{
    out.write(ptr, count);
}

long FontWriter::otfTell()
{
    return out.tellp();
}

void FontWriter::otfSeek(long offset)
{
    out.seekp(offset);
}

char *FontWriter::otfRefill(long *count)
{
    *count = otfdata.size();
    return &otfdata[0];
}

char *FontWriter::featOpen(char *name, long offset)
{
    if (!name) return nullptr;

    feature = std::ifstream(name, std::ios::binary);
    feature.seekg(offset);
    return name;
}

char *FontWriter::featRefill(long *count)
{
    *count = 8192;
    featuredata.resize(*count);
    feature.read(&featuredata[0], *count);
    return &featuredata[0];
}

void FontWriter::featClose()
{
    featuredata.clear();
    feature.close();
}

void FontWriter::featAddAnonData(char *, long, unsigned long)
{
    throw std::runtime_error("featAddAnonData unimplemented");
}

void FontWriter::tmpOpen()
{
    throw std::runtime_error("tmpOpen unimplemented");
}

void FontWriter::tmpWriteN(long, char *)
{
    throw std::runtime_error("tmpWriteN unimplemented");
}

void FontWriter::tmpRewind()
{
    throw std::runtime_error("tmpRewind unimplemented");
}

char *FontWriter::tmpRefill(long *)
{
    throw std::runtime_error("tmpRefill unimplemented");
}

void FontWriter::tmpClose()
{
    if (temp.is_open()) temp.close();
}

char *FontWriter::getFinalGlyphName(char *)
{
    throw std::runtime_error("getFinalGlyphName unimplemented");
}

char *FontWriter::getSrcGlyphName(char *)
{
    throw std::runtime_error("getSrcGlyphName unimplemented");
}

char *FontWriter::getUVOverrideName(char *)
{
    throw std::runtime_error("getUVOverrideName unimplemented");
}

char *FontWriter::uvsOpen(char *)
{
    throw std::runtime_error("uvsOpen unimplemented");
}

char *FontWriter::uvsGetLine(char *, long *)
{
    throw std::runtime_error("uvsGetLine unimplemented");
}

void FontWriter::uvsClose()
{
    throw std::runtime_error("uvsClose unimplemented");
}

char *FontWriter::CMapId()
{
    return const_cast<char*>("CMapId (unimplemented)");
}

char *FontWriter::CMapRefill(long *count)
{
    std::cout << "CMapRefill called" << std::endl;

    *count = cmapdata.size();
    return &cmapdata[0];
}

/* End of callbacks */

void FontWriter::convert()
{
    hotConvert(ctx);
}

void writeOTF(Font const& font, std::string filename)
{
    std::ifstream psfile("data/cidfont.ps.KR", std::ios::binary);
    if (!psfile.is_open()) throw std::runtime_error("error opening font file");
    std::string ps{
        std::istreambuf_iterator<char>(psfile),
        std::istreambuf_iterator<char>()
    };

    std::ifstream cmapfile("data/UniSourceHanSansKR-UTF32-H", std::ios::binary);
    if (!cmapfile.is_open()) throw std::runtime_error("error opening cmap file");
    std::string cmap{
        std::istreambuf_iterator<char>(cmapfile),
        std::istreambuf_iterator<char>()
    };

    FontWriter writer(filename, ps, cmap);
    writer.convert();
}

}
