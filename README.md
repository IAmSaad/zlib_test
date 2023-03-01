# Testing zlib for CERN's FASER project 
Evaluation program for testing zlib's capabilities.
Heavily based on https://www.zlib.net/zlib_how.html and zpipe.c, testzlib.c examples from zlib's github

## Info
Compresses an input file using zlib's two distinct compression levels namely `Z_DEFAULT_COMPRESSION` (provides optimal speed and compression tradeoff) and `Z_BEST_COMPRESSION` (gives best compression results), reports the output compressed file sizes and compression ratios along with the time taken to do both compressions.

## Usage
The /x64/Release/ directory contains the test program "zlib_build_test.exe" in it. 

Run the program from command line like this:

`zlib_build_test.exe -c "file_to_compress_with_extension" "name_of_compressed_output_file_with_extension"`

The `sample.txt` provided in the release folder can be used to test out this program, in that case to run the program the command will be :

`zlib_build_test.exe -c "sample.txt" "sample.z"` where sample.z is the output file's name. The .z extension is randomly given.

## About the Code
`zlib_build_test.cpp`: is the main file that has been modified from the zpipe.c example file. It contains all the functionality of the example program like command line handling, file handling, doing compression at two levels, printing compression statistics and timing etc.
