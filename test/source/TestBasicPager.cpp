#include <filesystem>
#include <random>

#include <catch2/catch_test_macros.hpp>

#include "../source/backend/pager.hpp"

class TestFixture
{
public:
  const std::string test_file = "test.db";

  void SetUp()
  {
    if (std::filesystem::exists(test_file)) {
      std::filesystem::remove(test_file);
    }
    // Create empty file
    std::ofstream file(test_file, std::ios::binary);
    std::vector<std::byte> initial_page(PAGE_SIZE, std::byte {0});
    file.write(reinterpret_cast<const char*>(initial_page.data()), PAGE_SIZE);
    file.close();
  }

  void TearDown()
  {
    if (std::filesystem::exists(test_file)) {
      if (std::filesystem::exists(test_file)) {
        std::filesystem::remove(test_file);
      }
    }
  }
};

TEST_CASE("Basic Pager Operations", "[pager]")
{
  TestFixture fixture;
  fixture.SetUp();

  SECTION("Initialization")
  {
    auto pager = create_pager("test.db");
    REQUIRE(pager != nullptr);
    REQUIRE(pager->get_num_pages() == 1);
  }

  SECTION("Page Read/Write")
  {
    auto pager = create_pager("test.db");
    auto page = pager->get_page(0);
    // Write pattern to page
    std::fill_n(page->data.begin(), PAGE_SIZE, std::byte {0xAA});
    page->is_dirty = true;

    // Write page back
    pager->write_page(*page);

    // Read back and verify
    auto read_page = pager->get_page(0);
    REQUIRE(read_page->data[0] == std::byte {0xAA});
  }

  fixture.TearDown();
}

TEST_CASE("Error Handling", "[pager]")
{
  TestFixture fixture;
  fixture.SetUp();

  SECTION("Invalid Page Access")
  {
    auto pager = create_pager("test.db");
    REQUIRE_THROWS_AS(pager->get_page(1000000), std::out_of_range);
  }

  SECTION("Invalid File Operations")
  {
    REQUIRE_THROWS(create_pager("/invalid/path/test.db"));
  }

  fixture.TearDown();
}

TEST_CASE("Cache Hit", "[pager]")
{
  TestFixture fixture;
  fixture.SetUp();

  auto pager = create_pager("test.db");

  auto page1 = pager->get_page(0);
  REQUIRE(pager->get_cache_hits() == 0);

  // Second access - should hit
  auto page2 = pager->get_page(0);
  REQUIRE(pager->get_cache_hits() == 1);

  fixture.TearDown();
}

TEST_CASE("File Persistence", "[pager]")
{
  TestFixture fixture;
  fixture.SetUp();

  {
    auto pager = create_pager("test.db");
    auto page = pager->get_page(0);
    std::fill_n(page->data.begin(), PAGE_SIZE, std::byte {0xBB});
    page->is_dirty = true;
    pager->write_page(*page);
  }

  {
    auto pager = create_pager("test.db");
    auto page = pager->get_page(0);
    REQUIRE(page->data[0] == std::byte {0xBB});
  }

  fixture.TearDown();
}