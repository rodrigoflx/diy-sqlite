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
 * @return std::uint32_t
 */
std::uint32_t Pager::get_num_pages() noexcept
{
  return num_pages;
}

/**
 * @brief Get a 4096KB page from the file in Pager
 *
 * @param page_number Page number
 * @return std::shared_ptr<Page>
 */
std::shared_ptr<Page> Pager::get_page(int page_number)
{
  if (page_number >= num_pages) {
    throw std::out_of_range("Page number out of bounds");
  }

  size_t cache_index = page_number % CACHE_SIZE;
  auto& entry = m_page_table[cache_index];

  /* If page cached in the slot is different, evict*/
  if (entry.is_valid && entry.stored_page_number != page_number) {
    evict_page(cache_index);
  }

  if (!entry.is_valid) {
    std::size_t offset = page_number * PAGE_SIZE;
    file_stream.seekg(offset);
    file_stream.read(reinterpret_cast<char*>(
                         cache.get() + (page_number % CACHE_PAGES) * PAGE_SIZE),
                     PAGE_SIZE);
    entry.stored_page_number = page_number;
    entry.is_valid = true;
  } else {
    cache_hits++;
  }

  std::vector<std::byte> page_data(PAGE_SIZE);
  std::memcpy(page_data.data(), cache.get() + cache_index * PAGE_SIZE, PAGE_SIZE);
  return std::make_shared<Page>(Page {
      page_number,
      false,
      std::move(page_data)});
}

void Pager::write_page(const Page& page)
{
  if (!page.is_dirty)
    return;

  // Update cache with new data
  std::memcpy(cache.get() + page.page_number * PAGE_SIZE, page.data.data(), PAGE_SIZE);

  std::size_t offset = page.page_number * PAGE_SIZE;
  file_stream.seekp(offset);
  file_stream.write(reinterpret_cast<const char*>(page.data.data()), PAGE_SIZE);
}

void Pager::flush(int page_number)
{
  if (page_number >= num_pages)
    return;

  std::size_t offset = page_number * PAGE_SIZE;
  std::size_t cache_offset = (page_number % CACHE_PAGES) * PAGE_SIZE;

  file_stream.seekp(offset);
  file_stream.write(reinterpret_cast<const char*>(cache.get() + cache_offset),
                    PAGE_SIZE);
  file_stream.flush();
}

Pager::~Pager()
{
  if (file_stream.is_open()) {
    file_stream.close();
  }
}

auto create_pager(const std::filesystem::path& filename)
    -> std::unique_ptr<Pager>
{
  return std::make_unique<Pager>(filename);
}

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