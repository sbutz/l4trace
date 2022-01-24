#pragma once

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

template<typename T>
class ExternalSort {
public:
    static const size_t B = 1UL;
    static const size_t KB = 1024 * B;
    static const size_t MB = 1024 * KB;
    static const size_t GB = 1024 * MB;

    static void sort(const std::string& path,
                     const std::string& out_file,
                     bool (*cmp)(const T &a, const T &b),
                     size_t max_mem);
};

template<typename T>
void ExternalSort<T>::sort(const std::string& in_file,
                           const std::string& out_file,
                           bool (*cmp)(const T &a, const T &b),
                           size_t max_mem)
{
    FILE *in = std::fopen(in_file.c_str(), "r");
    FILE *out = std::fopen(out_file.c_str(), "w");

    assert(in != nullptr);
    assert(out != nullptr);
    //assert(max_mem > X);
    assert(max_mem % sizeof(T) == 0);

    size_t file_size = std::filesystem::file_size(in_file);
    size_t n = file_size / max_mem + 1;


    /*
     * Phase 1: Split
     */
    std::vector<FILE *> tmpfiles(n);
    std::vector<T> buf(max_mem/sizeof(T));
    for (int i = 0; i < n; i++) {
        if (std::feof(in) || std::ferror(in))
            break;
        size_t read_c = std::fread(buf.data(), sizeof(T), buf.size(), in);
        std::sort(buf.begin(), std::next(buf.begin(), read_c), cmp);

        tmpfiles[i] = std::tmpfile();
        size_t write_c = std::fwrite(buf.data(), sizeof(T), read_c, tmpfiles[i]);
        assert(read_c == write_c);

        std::rewind(tmpfiles[i]);
    }

    /*
     * Phase 2: Merge
     */
    std::vector<std::vector<T>> bufs(n);
    std::vector<size_t> pos(n);
    for (int i = 0; i < n; i++) {
        bufs[i].resize(max_mem / sizeof(T) / n);
        size_t count = std::fread(bufs[i].data(), sizeof(T), bufs[i].size(), tmpfiles[i]);
        bufs[i].resize(count);
        pos[i] = 0;
    }

    while(!bufs.empty()) {
        size_t count;
        int i = 0;

        for (int buf_i = 1; buf_i < bufs.size(); buf_i++) {
            if (cmp(bufs[buf_i][pos[buf_i]], bufs[i][pos[i]]))
                i = buf_i;
        }

        count = std::fwrite(&bufs[i][pos[i]], sizeof(T), 1, out);
        assert(count == 1);

        pos[i] = pos[i]+1;
        if (pos[i] == bufs[i].size()) {
            count = std::fread(bufs[i].data(), sizeof(T), bufs[i].size(), tmpfiles[i]);
            bufs[i].resize(count);
            pos[i] = 0;

            if (count <= 0 && (std::feof(tmpfiles[i]) || std::ferror(tmpfiles[i]))) {
                std::fclose(tmpfiles[i]);
                tmpfiles.erase(std::next(tmpfiles.begin(), i));
                bufs.erase(std::next(bufs.begin(), i));
                pos.erase(std::next(pos.begin(), i));
            }
        }
    }
}
