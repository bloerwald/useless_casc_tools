#include <array>
#include <map>
#include <vector>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>

using hash = std::array<unsigned char, 0x10>;

std::string hash_to_str (hash const& h)
{
  std::ostringstream oss;
  oss << std::hex;
  for (size_t x : h)
  {
    oss << std::setfill ('0') << std::setw (2) << (x & 0xFF);
  }
  return oss.str();
}

int main (int argc, char** argv)
{
  std::string encoding_mini (argv[1]);
  std::string index_mini ("index.mini");

  FILE* index_file (fopen (index_mini.c_str(), "rb"));
  fseek (index_file, 0, SEEK_END);
  std::vector<std::pair<hash, hash>> index_v (ftell (index_file) / sizeof (std::pair<hash, hash>));
  fseek (index_file, 0, SEEK_SET);
  fread (index_v.data(), sizeof (decltype (index_v)::value_type), index_v.size(), index_file);
  fclose (index_file);

  FILE* encoding_file (fopen (encoding_mini.c_str(), "rb"));
  fseek (encoding_file, 0, SEEK_END);
  std::vector<std::pair<hash, hash>> encoding_v (ftell (encoding_file) / sizeof (std::pair<hash, hash>));
  fseek (encoding_file, 0, SEEK_SET);
  fread (encoding_v.data(), sizeof (decltype (encoding_v)::value_type), encoding_v.size(), encoding_file);
  fclose (encoding_file);

  std::map<hash, hash> index (index_v.begin(), index_v.end());
  std::multimap<hash, hash> encoding (encoding_v.begin(), encoding_v.end());

  for (auto& k : encoding)
  {
    try
    {
      auto&& x (index.at (k.second));
      std::cout << "ARCHIVE " << hash_to_str (x) << "\n";
    }
    catch (...)
    {
      std::cout << "UNKNOWN " << hash_to_str (k.second) << "\n";
    }
  }



  return 0;
}
