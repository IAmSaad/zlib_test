// zlib_build_test.cpp
// Author: Saad Mahmood

// Heavily based on https://www.zlib.net/zlib_how.html and zpipe.c, testzlib.c examples from zlib's github

// Edit 27/02/2023: Added _CRT_SECURE_NO_WARNINGS in preprocessor definitions to ignore numerous warnings
// Edit 27/02/2023: Changed inputs of def() from stdin/stdout to filePtrs,
//                  also needed to change how argv[2] and argv[3] are passed to def() function using fopen
// Edit 27/02/2023: Found VStudio finds VS cmdline argument files in project working directory and also writes to the same
// Edit 28/02/2023: Enabled VC++17 support to use std::filesystem
// Edit 01/03/2023: Added timing capability for measuring zlib's BEST_COMPRESSION and DEFAULT_COMPRESSION levels

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <filesystem>
#include <chrono>
#include "zlib.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

    /* Compress from file source to file dest until EOF on source.
       def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
       allocated for processing, Z_STREAM_ERROR if an invalid compression
       level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
       version of the library linked do not match, or Z_ERRNO if there is
       an error reading or writing the files. */
int def(FILE* source, FILE* dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE* source, FILE* dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
int main(int argc, char** argv)
{
    int ret = 0;

    printf("Simple zlib evaluation program by Saad for CERN GSoC evaluation\n\n");

    /* avoid end-of-line conversions */
    SET_BINARY_MODE(stdin);
    SET_BINARY_MODE(stdout);

    /* do nothing if no arguments */
    if (argc == 1) {
        printf("No cmd line arguments were provided!\n");
        printf("Usage:\n");
        printf("zlib_build_test.exe -c file_to_compress.txt compressed_output_file.z");
        return 0;
    }

    /* do compression if -c specified */
    else if (argc == 4 && strcmp(argv[1], "-c") == 0) {
        //make string filenames for all files and append marker string to denote default compression
        //or best compression to the respective files
        std::string input_file = std::string(argv[2]);
        std::string output_file1 = std::string(argv[3]) + "_default_compression";
        std::string output_file2 = std::string(argv[3]) + "_best_compression";

        //make file pointers to input and output files
        FILE * pfileToCompress = fopen(input_file.c_str(), "rb");
        FILE * pfileToWriteCompressedDefaultComp = fopen(output_file1.c_str(), "wb");
        FILE* pfileToWriteCompressedBestComp = fopen(output_file2.c_str(), "wb");

        //throw error if unable to read input or create output files
        if (pfileToCompress == NULL || pfileToWriteCompressedDefaultComp == NULL
            || pfileToWriteCompressedBestComp == NULL)
        {
            printf("Unable to read input file or create output files");
            return -1;
        }

        //variables for run-time keeping  
        std::chrono::steady_clock::time_point default_comp_time_start;
        auto best_comp_time_start = default_comp_time_start;
        auto default_comp_time_finish = default_comp_time_start;
        auto best_comp_time_finish = default_comp_time_start;

        printf("Doing compression...");

        //start timer for def() call with DEFAULT_COMPRESSION
        default_comp_time_start = std::chrono::high_resolution_clock::now();
        //use deflate() method with DEFAULT_COMPRESSION
        ret = def(pfileToCompress, pfileToWriteCompressedDefaultComp, Z_DEFAULT_COMPRESSION);
        //end timer for def() call with DEFAULT_COMPRESSION
        default_comp_time_finish = std::chrono::high_resolution_clock::now();

        if (ret == Z_OK)
        {
            //reset file pointer to start of file
            fseek(pfileToCompress, 0, SEEK_SET);
            
            //start timer for def() call with BEST_COMPRESSION
            best_comp_time_start = std::chrono::high_resolution_clock::now();
            //use deflate() method with BEST_COMPRESSION
            ret = def(pfileToCompress, pfileToWriteCompressedBestComp, Z_BEST_COMPRESSION);
            //end timer for def() call with BEST_COMPRESSION
            best_comp_time_finish = std::chrono::high_resolution_clock::now();
        }
        
        printf("Done!\n");

        //close input and output files
        fclose(pfileToCompress);
        fclose(pfileToWriteCompressedDefaultComp);
        fclose(pfileToWriteCompressedBestComp);

        //if unable to do compression
        if (ret != Z_OK)
            zerr(ret);
        //otherwise print compression statistics
        else
        {
            //calculate milliseconds taken
            auto default_comp_time = std::chrono::duration_cast<std::chrono::milliseconds>\
                (default_comp_time_finish - default_comp_time_start);
            auto best_comp_time = std::chrono::duration_cast<std::chrono::milliseconds>\
                (best_comp_time_finish - best_comp_time_start);

            //calculate file sizes
            std::filesystem::path source_file = input_file;
            std::filesystem::path dest_file_def_comp = output_file1;
            std::filesystem::path dest_file_best_comp = output_file2;

            auto source_file_sz = std::filesystem::file_size(source_file);
            auto dest_def_comp_file_sz = std::filesystem::file_size(dest_file_def_comp);
            auto dest_best_comp_file_sz = std::filesystem::file_size(dest_file_best_comp);
                        
            printf("------------------------------------------------------------\n");
            printf("Compression Statistics :-\n");
            printf("Original file size is %d bytes\n", (uint32_t)source_file_sz);
            printf("Default compression took %d milliseconds and produced %d bytes (%0.2f compression ratio)\n", \
                (int) default_comp_time.count(), (uint32_t)dest_def_comp_file_sz, \
                (float)source_file_sz / dest_def_comp_file_sz);
            printf("Best compression took %d milliseconds and produced %d bytes (%0.2f compression ratio)\n", \
                (int) best_comp_time.count(), (uint32_t)dest_best_comp_file_sz, \
                (float)source_file_sz / dest_best_comp_file_sz);
            printf("------------------------------------------------------------\n");
        }

        return ret;
    }

    /* otherwise, report usage */
    else {
        printf("Usage:\n");
        fputs("zlib_build_test.exe -c file_to_compress.txt compressed_output_file.z",stderr);
        return 1;
    }
}

