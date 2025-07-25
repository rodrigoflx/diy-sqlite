#ifndef PAGER_HPP
#define PAGER_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <vector>

constexpr std::size_t PAGE_SIZE = 4096;
constexpr std::size_t CACHE_PAGES = 100;

class Page
{
public:
  int page_number;
  bool is_dirty;
  std::vector<std::byte> data;
};

class Pager
{
public:
  // Constructor/Destructor
  explicit Pager(
      const std::filesystem::path& filename);  // Can throw runtime_error
  ~Pager();

  // Delete copy operations
  Pager(const Pager&) = delete;
  Pager& operator=(const Pager&) = delete;

  // Allow move operations
  Pager(Pager&&) noexcept = default;
  Pager& operator=(Pager&&) noexcept = default;

  // Page operations
  void flush(int page_number);
  std::shared_ptr<Page> get_page(int page_number);
  void write_page(const Page& page);

  // Getters
  std::uint32_t get_num_pages() noexcept;
  std::size_t get_cache_hits() const noexcept { return cache_hits; }

private:
  std::fstream file_stream;
  std::unique_ptr<std::byte[]> cache;
  std::uint32_t page_size;
  std::uint32_t file_size;
  std::uint32_t num_pages;
  std::size_t cache_hits {0};

  struct CacheEntry
  {
    int stored_page_number {-1};
    bool is_dirty {false};
    bool is_valid {false};
  };
  std::array<CacheEntry, CACHE_PAGES> m_page_table;
  void evict_page(size_t cache_index);
};

// Factory function
auto create_pager(const std::filesystem::path& filename)
    -> std::unique_ptr<Pager>;

#endif  // PAGER_HPP