
#include <chrono>
#include <fstream>
#include <gtest/gtest.h>
#include <pugixml.hpp>

#include "fontutils/cffutils.hpp"
#include "fontutils/tables/offsettable.hpp"

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

TEST(token_writer, fontutils)
{
    fontutils::Buffer buf1, buf2;
    fontutils::write_token(buf1, -2.25);
    auto t1 = fontutils::next_token(buf1);
    EXPECT_EQ(t1.get_type(), fontutils::CFFToken::Type::floating);
    EXPECT_DOUBLE_EQ(t1.to_double(), -2.25);

    fontutils::write_token(buf2, 0.140541E-3);
    auto t2 = fontutils::next_token(buf2);
    EXPECT_EQ(t2.get_type(), fontutils::CFFToken::Type::floating);
    EXPECT_DOUBLE_EQ(t2.to_double(), 0.140541E-3);

    for (int i = -2000; i < 2000; ++i)
    {
        fontutils::Buffer buf;
        fontutils::write_token(buf, i);
        auto t = fontutils::next_token(buf);
        EXPECT_EQ(t.get_type(), fontutils::CFFToken::Type::integer);
        EXPECT_DOUBLE_EQ(t.to_int(), i);
    }

    {
        fontutils::Buffer buf;
        fontutils::write_token(buf, -12312312);
        auto t = fontutils::next_token(buf);
        EXPECT_EQ(t.get_type(), fontutils::CFFToken::Type::integer);
        EXPECT_DOUBLE_EQ(t.to_int(), -12312312);
    }

    {
        fontutils::Buffer buf;
        fontutils::write_token(buf, 12312312);
        auto t = fontutils::next_token(buf);
        EXPECT_EQ(t.get_type(), fontutils::CFFToken::Type::integer);
        EXPECT_DOUBLE_EQ(t.to_int(), 12312312);
    }
}

TEST(write_font, fontutils)
{
    {
        std::ifstream file("data/NotoSansCJKkr-Regular.otf");
        fontutils::Buffer buf(std::string{std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>() });

        fontutils::OffsetTable table;
        table.parse(buf);

        std::ofstream otf("data/testout.otf");
        auto outbuf = table.compile();
        otf.write(outbuf.data(), outbuf.size());

        std::cout << "OTF file Succesfully written.\n" << std::endl;
    }

    std::ifstream test_file("data/testout.otf");
    fontutils::Buffer test_buf(std::string{ std::istreambuf_iterator<char>(test_file),
        std::istreambuf_iterator<char>() });

    fontutils::OffsetTable test_table;
    test_table.parse(test_buf);
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
