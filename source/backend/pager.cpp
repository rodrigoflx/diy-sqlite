#include <cstring>
#include <system_error>

#include "pager.hpp"

// Constants
constexpr std::size_t CACHE_SIZE = PAGE_SIZE * CACHE_PAGES;

/**
 * @brief Construct a new Pager::Pager object
 *
 * @param filename Path to the file
 * @throws std::runtime_error if file cannot be opened
 */
Pager::Pager(const std::filesystem::path& filename)
    : page_size(PAGE_SIZE)
    , cache(std::make_unique<std::byte[]>(CACHE_SIZE))
{
  file_stream.open(filename, std::ios::binary | std::ios::in | std::ios::out);
  if (!file_stream) {
    throw std::runtime_error("Cannot open file");
  }

  // Get file size
  file_stream.seekg(0, std::ios::end);
  file_size = file_stream.tellg();
  num_pages = (file_size + page_size - 1) / page_size;
}

/**
 * @brief Get the number of pages in the file
 *
 * @return Number of pages in the database file
 */
std::uint32_t Pager::get_num_pages() noexcept
{
  return num_pages;
}

/**
 * @brief Retrieve a page from the cache or load it from disk if not cached
 *
 * @param page_number The page number to retrieve
 * @return std::shared_ptr<Page> The requested page
 * @throws std::out_of_range if page_number is out of bounds
 */
std::shared_ptr<Page> Pager::get_page(int page_number)
{
  if (page_number >= num_pages) {
    throw std::out_of_range("Page number out of bounds");
  }

  size_t cache_index =
      page_number % CACHE_PAGES;  // FIX: use CACHE_PAGES not CACHE_SIZE
  auto& entry = m_page_table[cache_index];

  /* If page cached in the slot is different, evict*/
  if (entry.is_valid && entry.stored_page_number != page_number) {
    evict_page(cache_index);
  }

  if (!entry.is_valid) {
    std::size_t offset = page_number * PAGE_SIZE;
    file_stream.seekg(offset);
    file_stream.read(
        reinterpret_cast<char*>(cache.get() + cache_index * PAGE_SIZE),
        PAGE_SIZE);
    entry.stored_page_number = page_number;
    entry.is_valid = true;
    entry.is_dirty = false;
  } else {
    cache_hits++;
  }

  std::vector<std::byte> page_data(PAGE_SIZE);
  std::memcpy(
      page_data.data(), cache.get() + cache_index * PAGE_SIZE, PAGE_SIZE);
  return std::make_shared<Page>(
      Page {page_number, false, std::move(page_data)});
}

/**
 * @brief Write a page to the cache and disk if it is marked dirty
 *
 * @param page The page to write
 */
void Pager::write_page(const Page& page)
{
  if (!page.is_dirty)
    return;

  size_t cache_index = page.page_number % CACHE_PAGES;  // FIX: use CACHE_PAGES
  // Update cache with new data
  std::memcpy(
      cache.get() + cache_index * PAGE_SIZE, page.data.data(), PAGE_SIZE);
  m_page_table[cache_index].is_dirty = false;  // Mark as clean after write

  std::size_t offset = page.page_number * PAGE_SIZE;
  file_stream.seekp(offset);
  file_stream.write(reinterpret_cast<const char*>(page.data.data()), PAGE_SIZE);
}

/**
 * @brief Flush a page from the cache to disk
 *
 * @param page_number The page number to flush
 */
void Pager::flush(int page_number)
{
  if (page_number >= num_pages)
    return;

  size_t cache_index = page_number % CACHE_PAGES;  // FIX: use CACHE_PAGES
  std::size_t offset = page_number * PAGE_SIZE;
  file_stream.seekp(offset);
  file_stream.write(
      reinterpret_cast<const char*>(cache.get() + cache_index * PAGE_SIZE),
      PAGE_SIZE);
  file_stream.flush();
}

/**
 * @brief Destructor for Pager, closes the file stream
 */
Pager::~Pager()
{
  if (file_stream.is_open()) {
    file_stream.close();
  }
}

/**
 * @brief Factory function to create a Pager instance
 *
 * @param filename Path to the database file
 * @return std::unique_ptr<Pager> New Pager instance
 */
auto create_pager(const std::filesystem::path& filename)
    -> std::unique_ptr<Pager>
{
  return std::make_unique<Pager>(filename);
}

/**
 * @brief Evict a page from the cache, writing it to disk if dirty
 *
 * @param cache_index Index of the cache slot to evict
 */
void Pager::evict_page(size_t cache_index)
{
  auto& entry = m_page_table[cache_index];
  if (entry.is_valid && entry.is_dirty) {
    std::size_t offset = entry.stored_page_number * PAGE_SIZE;
    file_stream.seekp(offset);
    file_stream.write(
        reinterpret_cast<const char*>(cache.get() + cache_index * PAGE_SIZE),
        PAGE_SIZE);
  }
  entry.is_valid = false;
  entry.is_dirty = false;
}