#include "fmt/format.h"
#include <errno.h>
#include <hs.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace {
    struct SmallPolicy {
        SmallPolicy(const std::string &patt) : pattern(patt) {
            hs_compile_error_t *compile_err;
            unsigned int flags = HS_FLAG_DOTALL | HS_FLAG_SOM_LEFTMOST;
            unsigned int regex_mode = HS_MODE_BLOCK;
            auto errcode = hs_compile(pattern.data(), flags, regex_mode, nullptr, &database, &compile_err);
            if (errcode != HS_SUCCESS) {
                const std::string errmsg = "ERROR: Unable to compile pattern " + pattern + ":" + compile_err->message;
                hs_free_compile_error(compile_err);
                throw std::runtime_error(errmsg);
            }

            if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
                hs_free_database(database);
                throw(std::runtime_error("ERROR: Unable to allocate scratch space for hyperscan."));
            }
        }

        void operator()(const char *begin, const char *end) {
            auto event_handler = [](unsigned int, unsigned long long, unsigned long long to, unsigned int,
                                    void *ctx) {
                printf("Match for pattern \"%s\" at offset %llu\n", (char *)ctx, to);
                return HS_SUCCESS;
            };

            if (hs_scan(database, begin, end - begin, 0, scratch, event_handler, (void*)pattern.data()) != HS_SUCCESS) {
                fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
            }
        }

        ~SmallPolicy() {
            if (database) hs_free_database(database);
            if (scratch) hs_free_scratch(scratch);
        }

        std::string pattern;
        hs_database_t *database = nullptr;
        hs_scratch_t *scratch = nullptr;
    };

    /**
     * Fill a data buffer from the given filename, returning it and filling @a
     * length with its length. Returns NULL on failure.
     */
    char *readInputData(const char *inputFN, unsigned int *length) {
        FILE *f = fopen(inputFN, "rb");
        if (!f) {
            fprintf(stderr, "ERROR: unable to open file \"%s\": %s\n", inputFN, strerror(errno));
            return NULL;
        }

        /* We use fseek/ftell to get our data length, in order to keep this example
         * code as portable as possible. */
        if (fseek(f, 0, SEEK_END) != 0) {
            fprintf(stderr, "ERROR: unable to seek file \"%s\": %s\n", inputFN, strerror(errno));
            fclose(f);
            return NULL;
        }
        long dataLen = ftell(f);
        if (dataLen < 0) {
            fprintf(stderr, "ERROR: ftell() failed: %s\n", strerror(errno));
            fclose(f);
            return NULL;
        }
        if (fseek(f, 0, SEEK_SET) != 0) {
            fprintf(stderr, "ERROR: unable to seek file \"%s\": %s\n", inputFN, strerror(errno));
            fclose(f);
            return NULL;
        }

        /* Hyperscan's hs_scan function accepts length as an unsigned int, so we
         * limit the size of our buffer appropriately. */
        if ((unsigned long)dataLen > UINT_MAX) {
            dataLen = UINT_MAX;
            printf("WARNING: clipping data to %ld bytes\n", dataLen);
        } else if (dataLen == 0) {
            fprintf(stderr, "ERROR: input file \"%s\" is empty\n", inputFN);
            fclose(f);
            return NULL;
        }

        char *inputData = (char *)malloc(dataLen);
        if (!inputData) {
            fprintf(stderr, "ERROR: unable to malloc %ld bytes\n", dataLen);
            fclose(f);
            return NULL;
        }

        char *p = inputData;
        size_t bytesLeft = dataLen;
        while (bytesLeft) {
            size_t bytesRead = fread(p, 1, bytesLeft, f);
            bytesLeft -= bytesRead;
            p += bytesRead;
            if (ferror(f) != 0) {
                fprintf(stderr, "ERROR: fread() failed\n");
                free(inputData);
                fclose(f);
                return NULL;
            }
        }

        fclose(f);

        *length = (unsigned int)dataLen;
        return inputData;
    }
} // namespace

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pattern> <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *pattern = argv[1];
    char *inputFN = argv[2];

    /* Next, we read the input data file into a buffer. */
    unsigned int length;
    char *inputData = readInputData(inputFN, &length);
    if (!inputData) { return EXIT_FAILURE; }

    printf("Scanning %u bytes with Hyperscan\n", length);
    SmallPolicy grep(pattern);
    grep(inputData, inputData + length);

    free(inputData);
    return EXIT_SUCCESS;
}
