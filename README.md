# zlib_build_test
Evaluation program for testing zlib's capabilities.
Heavily based on https://www.zlib.net/zlib_how.html and zpipe.c, testzlib.c examples from zlib's github

## Usage
The /x64/Release/ directory contains the test program "zlib_build_test.exe" in it. 

Run the program from command line like this:

`zlib_build_test.exe -c "file_to_compress_with_extension" "name_of_compressed_output_file_with_extension"`

## Info
Compresses an input file using zlib's two distinct compression levels namely `Z_DEFAULT_COMPRESSION` and `Z_BEST_COMPRESSION`, reports the output compressed file sizes and compression ratios along with the time taken to do both compressions
