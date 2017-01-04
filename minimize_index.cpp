#include <vector>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>
#include <map>

#include "blte.hpp"
#include "bigendian.hpp"

struct index_entry
{
  std::array<unsigned char, 16> hash;
  uint32_BE_t size;
  uint32_BE_t offset;

  bool has_data() const
  {
    return std::any_of (hash.begin(), hash.end(), [](unsigned char c) { return !!c; });
  }
  friend std::ostream& operator<< (std::ostream& os, index_entry const& e)
  {
    auto f (os.flags());
    os << std::hex;
    for (size_t x : e.hash)
    {
      os << std::setfill ('0') << std::setw (2) << (x & 0xFF);
    }
    os << " " << std::setw (8) << e.offset;
    os << " " << std::setw (8) << e.size;
    os.flags (f);
    return os;
  }

  std::string hash_as_str() const
  {
    std::ostringstream oss;
    oss << std::hex;
    for (size_t x : hash)
    {
      oss << std::setfill ('0') << std::setw (2) << (x & 0xFF);
    }
    return oss.str();
  }
};

struct index_block
{
  std::array<index_entry, 0x1000 / sizeof (index_entry)> entries;
  char padding[0x1000 - sizeof (entries)];
};

int main (int argc, char** argv)
{
  FILE* b (fopen ("index.mini", "wb+"));

  std::map<std::array<unsigned char, 0x10>, std::array<unsigned char, 0x10>> keys;

  for (int arg (1); arg < argc; ++arg)
  {
    std::string blob (argv[arg]);
    std::array<unsigned char, 0x10> blob_key;

    for (unsigned char& y : blob_key)
    {
      std::string sub (blob.substr (2 * (&y - blob_key.data()), 2));
      size_t x;
      sscanf(sub.c_str(), "%2lx", &x );
      y = x;
    }

    std::string index (blob + ".index");
    FILE* f (fopen (index.c_str(), "rb"));
    fseek (f, 0, SEEK_END);
    std::vector<index_block> data (ftell (f) / sizeof (index_block));
    fseek (f, 0, SEEK_SET);
    fread (data.data(), sizeof (index_block), data.size(), f);
    fclose (f);

    for (auto& block : data)
    {
      for (auto& entry : block.entries)
      {
        if (entry.has_data())
        {
          keys.emplace (entry.hash, blob_key);
        }
      }
    }
  }

  for (auto& key : keys)
  {
    fwrite (&key, sizeof (key), 1, b);
  }

  fclose (b);
  return 0;
}
