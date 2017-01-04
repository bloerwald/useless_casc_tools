#include "bigendian.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <cstdio>
#include <vector>

#pragma pack(1)

struct EN_header {
  char E;char N;
  char unk_2;
  char checksumSizeA;
  char checksumSizeB;
  uint16_BE_t flagsA;
  uint16_BE_t flagsB;
  uint32_BE_t entriesA;
  uint32_BE_t entriesB;
  char unk_11;
  uint32_BE_t stringBlockSize;

  void validate() const
  {
    if (E != 'E' || N != 'N' || checksumSizeA != 0x10 || checksumSizeB != 0x10)
    {
      throw std::logic_error ("implementation unable to cope with this file");
    }
  }
};

struct entry_block {
  struct entry {
    uint16_t key_count;
    uint32_BE_t fileSize;
    std::array<unsigned char, 16> hash;
    std::array<unsigned char, 16> key;

    bool has_data() const
    {
      return std::any_of (hash.begin(), hash.end(), [](unsigned char c) { return !!c; });
    }

    std::vector<std::array<unsigned char, 16>> keys() const
    {
      auto keys (&key);
      std::vector<std::array<unsigned char, 16>> ret;
      for (int i(0); i < key_count; ++i )
      {
        ret.emplace_back (keys[i]);
      }
      return ret;
    }

    entry const* next() const
    {
      char const* x (reinterpret_cast<char const*> (this));
      return reinterpret_cast<entry const*> (x + 2 + 4 + 16 + 16 * key_count);
    }
  };

  std::array<entry, 4096 / sizeof (entry)> entries;
  char padding[4096 - sizeof (entries)];
};

int main (int argc, char** argv)
{
  FILE* f (fopen (argv[1], "rb"));
  fseek (f, 0, SEEK_END);
  std::vector<char> data (ftell (f));
  fseek (f, 0, SEEK_SET);
  fread (data.data(), 1, data.size(), f);
  fclose (f);

  std::multimap<std::array<unsigned char, 0x10>, std::array<unsigned char, 0x10>> keys;

  EN_header const* header (reinterpret_cast<EN_header const*> (data.data()));
  header->validate();
  entry_block const* entry_blocks ( reinterpret_cast<entry_block const*>
                                      ( data.data()
                                      + sizeof (EN_header)
                                      + (uint32_t) header->stringBlockSize
                                      + (uint32_t) header->entriesA * 0x20
                                      )
                                  );

  for (uint32_t i (0); i < header->entriesA; ++i)
  {
    entry_block::entry const* e (entry_blocks[i].entries.data());
    while (e < entry_blocks[i].entries.end())
    {
      if (e->has_data())
      {
        for (auto& k : e->keys())
        {
          keys.emplace (e->hash, k);
        }
      }
      e = e->next();
    }
  }

  FILE* b (fopen ((argv[1] + std::string (".mini")).c_str(), "wb+"));

  for (auto& key : keys)
  {
    fwrite (&key, sizeof (key), 1, b);
  }

  fclose (b);

  return 0;
}
