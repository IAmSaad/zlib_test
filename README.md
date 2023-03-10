# Benchmarking zlib
Evaluation program (Windows only at this time) for testing zlib's capabilities.
Heavily based on https://www.zlib.net/zlib_how.html and zpipe.c, testzlib.c examples from zlib's github repository.

## Info
Compresses an input file using zlib's two distinct compression levels namely `Z_DEFAULT_COMPRESSION` (optimal speed/compression tradeoff) and `Z_BEST_COMPRESSION` (best compression ratio), reports the output compressed file sizes and compression ratios along with the time taken to do both compressions.

## Usage
The /x64/Release/ directory contains the test program "zlib_build_test.exe" in it. 

Run the program from command line like this:

`zlib_build_test.exe -c "file_to_compress_with_extension" "name_of_compressed_output_file_with_extension"`

The "sample.txt" provided in the same folder (/release) can be used to test out this program, in this case to run the program the command will be :

`zlib_build_test.exe -c "sample.txt" "sample.z"` 

Here sample.z is the output file's name. The .z extension is randomly given and can be alphanumeric combination.

## Results
Using this executable with the provided sample.txt file, I get the following results
```
Original file size is 507421 bytes
Default compression took 33 milliseconds and produced 177141 bytes (2.86 compression ratio)
Best compression took 48 milliseconds and produced 176172 bytes (2.88 compression ratio)
```

## Limitations of current implementation
The statistics like compression time can vary as the compression is not done multiple times and averaged, and thus time especially varies (mostly in the first iteration). Multiple compression runs and averaging compression time is planned in future

## About the Code
`zlib_build_test.cpp`: is the main file that has been modified from the zlib/example/zpipe.c. It contains all the functionality of the example program like command line argument handling, file handling, doing compression at two levels, printing compression statistics and timing etc.

## Building the Code
Download the .zip file, extract and open the *"zlib_build_test.sln"* using *Visual Studio 2022 Community Edition*. Once it is opened, build the project. The built executable zlib_build_test.exe will be in the x64/Release folder
