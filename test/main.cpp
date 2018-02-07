
#include <gtest/gtest.h>
#include <pugixml.hpp>
#include <chrono>
#include <fstream>

#include "fontutils/tables/offsettable.hpp"
#include "fontutils/tables/unicodemap.hpp"

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

TEST(unicodemap, fontutils)
{
    using namespace fontutils;

    UnicodeMap<int> map;

    for (int i = 0; i < 1000; ++i)
        ASSERT_FALSE(map.is_set(i));

    map.set(5, 2000);

    for (int i = 0; i <= 4; ++i)
        ASSERT_FALSE(map.is_set(i));
    ASSERT_TRUE(map.is_set(5));
    ASSERT_EQ(map[5], 2000);
    for (int i = 6; i < 1000; ++i)
        ASSERT_FALSE(map.is_set(i));

    map.set(5, 3000);

    for (int i = 0; i <= 4; ++i)
        ASSERT_FALSE(map.is_set(i));
    ASSERT_TRUE(map.is_set(5));
    ASSERT_EQ(map[5], 3000);
    for (int i = 6; i < 1000; ++i)
        ASSERT_FALSE(map.is_set(i));

    map.set_range(3, 3000, -20);

    for (int i = 0; i <= 2; ++i)
        ASSERT_FALSE(map.is_set(i));
    for (int i = 3; i <= 2999; ++i)
    {
        ASSERT_TRUE(map.is_set(i));
        ASSERT_EQ(map[i], -20);
    }
    for (int i = 3000; i < 10000; ++i)
        ASSERT_FALSE(map.is_set(i));

    map.set_range(3, 2000, 32000);

    for (int i = 0; i <= 2; ++i)
        ASSERT_FALSE(map.is_set(i));
    for (int i = 3; i <= 1999; ++i)
    {
        ASSERT_TRUE(map.is_set(i));
        ASSERT_EQ(map[i], 32000);
    }
    for (int i = 2000; i <= 2999; ++i)
    {
        ASSERT_TRUE(map.is_set(i));
        ASSERT_EQ(map[i], -20);
    }
    for (int i = 3000; i < 10000; ++i)
        ASSERT_FALSE(map.is_set(i));

    map.set_range(1500, 3000, 15300);

    for (int i = 0; i <= 2; ++i)
        ASSERT_FALSE(map.is_set(i));
    for (int i = 3; i <= 1499; ++i)
    {
        ASSERT_TRUE(map.is_set(i));
        ASSERT_EQ(map[i], 32000);
    }
    for (int i = 1500; i <= 2999; ++i)
    {
        ASSERT_TRUE(map.is_set(i));
        ASSERT_EQ(map[i], 15300);
    }
    for (int i = 3000; i < 10000; ++i)
        ASSERT_FALSE(map.is_set(i));
}

TEST(write_font, fontutils)
{
    try
    {
        std::ifstream file("data/SourceHanSansKR-Regular.otf");
        //std::ifstream file("data/testout.otf");
        fontutils::Buffer buf(std::string{std::istreambuf_iterator<char>(file),
                                          std::istreambuf_iterator<char>()});

        fontutils::OffsetTable table;
        table.parse(buf);

        std::ofstream otf("data/testout.otf");
        auto outbuf = table.compile();
        otf.write(outbuf.data(), outbuf.size());
    }
    catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}

#if 0
TEST(open_file, ttx)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("data/NotoSansCJK-Medium.ttx");
    GTEST_ASSERT_EQ(result.status, pugi::status_ok);

    auto body = doc.child("ttFont");

    std::cout << "Before: \n";
    for (auto item : body) {
        size_t n = std::distance(item.begin(), item.end());
        std::cout << item.name() << ": " << n << '\n';
    }

    // remove all namerecords
    auto name_table = body.child("name");
    while (name_table.remove_child("namerecord"));

    /* Add namerecords platformID = 1 */ {
        auto copyright = name_table.append_child("namerecord");
        copyright.append_attribute("nameID") = 0;
        copyright.append_attribute("platformID") = 1;
        copyright.append_attribute("platEncID") = 0;
        copyright.append_attribute("langID") = 0x0;
        copyright.append_attribute("unicode") = "True";
        copyright.text() = "No Copyright";

        auto font_name = name_table.append_child("namerecord");
        font_name.append_attribute("nameID") = 1;
        font_name.append_attribute("platformID") = 1;
        font_name.append_attribute("platEncID") = 0;
        font_name.append_attribute("langID") = 0x0;
        font_name.append_attribute("unicode") = "True";
        font_name.text() = "Test Font Template";
    }

    /* Add namerecords platformID = 3 */ {
        auto copyright = name_table.append_child("namerecord");
        copyright.append_attribute("nameID") = 0;
        copyright.append_attribute("platformID") = 3;
        copyright.append_attribute("platEncID") = 1;
        copyright.append_attribute("langID") = 0x409;
        copyright.text() = "No Copyright";

        auto font_name = name_table.append_child("namerecord");
        font_name.append_attribute("nameID") = 1;
        font_name.append_attribute("platformID") = 3;
        font_name.append_attribute("platEncID") = 1;
        font_name.append_attribute("langID") = 0x409;
        font_name.text() = "Test Font Template";
    }

    auto CFF_font = body.child("CFF").child("CFFFont");
    CFF_font.attribute("name") = "TestFontName";
    for (auto item : CFF_font.child("FDArray")) {
        std::cout << item.child("FontName").attribute("value").as_string() << "\n";
    }

    std::cout << "\n";
    std::cout << "After: \n";
    for (auto item : body) {
        size_t n = std::distance(item.begin(), item.end());
        std::cout << item.name() << ": " << n << '\n';
    }

    auto save_ttx_name = "data/modified_font.ttx";
    GTEST_ASSERT_EQ(doc.save_file(save_ttx_name), true);

    // Flush stdout buffer
    std::cout << std::endl;
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    // Launch FontTools to compile xml into otf.
    auto save_otf_name = "data/modified_font.otf";
    std::remove(save_otf_name);
    std::string cmd = std::string("ttx -y 1 -o ") + save_otf_name + " " + save_ttx_name;
    int compile_result = std::system(cmd.c_str());

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
    std::cout << "compile_result : " << compile_result << ", took " << duration << " seconds.\n";
}
#endif
