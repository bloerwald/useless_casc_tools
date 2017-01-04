* `./minimize_encoding decoded-encoding-file`
* `ls *.index | sed -e s,.index,, | xargs ./minimize_index`
* `./minimize_encoding decoded-encoding-file.mini`

outputs `ARCHIVE archive_hash` for files from encoding found in any known archive and `UNKNOWN encoded_hash` for all others
