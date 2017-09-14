/*
 	  This Source Code Form is subject to the
      terms of the Mozilla Public License, v.
      2.0. If a copy of the MPL was not
      distributed with this file, You can
      obtain one at
      http://mozilla.org/MPL/2.0/.
 */
#ifndef MEMORY_MAPPED_FILE_HPP
#define MEMORY_MAPPED_FILE_HPP

#include <cstddef> // for size_t
#include <AlloyFileUtil.h>
namespace aly
{
    unsigned int mmf_granularity();

    class BaseMemMapFile
    {
    public:
        explicit BaseMemMapFile();
        ~BaseMemMapFile();
        size_t offset() const { return offset_; }
        size_t mapped_size() const { return mapped_size_; }
        size_t file_size() const { return file_size_; }
        void unmap();
        void close();
        bool is_open() const
        {
            return file_handle_ !=
    #ifdef ALY_WINDOWS
            (void*)
    #endif
            -1;
        }        
	#ifdef ALY_WINDOWS
        typedef void* HANDLE;
    #else
        typedef int HANDLE;
    #endif
        HANDLE file_handle() const
        {
            return file_handle_;
        }
    protected:
        size_t query_file_size_();
        char* data_;
        size_t offset_;
        size_t mapped_size_;
        size_t file_size_;
        int granularity_;
        HANDLE file_handle_;
	#ifdef ALY_WINDOWS
        HANDLE file_mapping_handle_;
    #endif
    };

    class ReadableMemMapFile: public BaseMemMapFile
    {
    public:
        explicit ReadableMemMapFile(char const* pathname = 0, bool map_all = true);
        ReadableMemMapFile(const std::string& path, bool map_all = true):ReadableMemMapFile(path.c_str(),map_all){
        }
        void open(char const* pathname, bool map_all = true);
        inline void open(const std::string& path, bool map_all = true){
        	open(path.c_str(),map_all);
        }

        char const* data() const { return data_; }
        void map(size_t offset = 0, size_t size = 0);
    };

    enum class ExistsPolicyMMF
    {
        if_exists_fail=0,
        if_exists_just_open=1,
        if_exists_map_all=2,
        if_exists_truncate=3
    };

    enum class DoesNotExistPolicyMMF
    {
        if_doesnt_exist_fail=0,
        if_doesnt_exist_create=1
    };
    class WriteableMemMapFile: public BaseMemMapFile
    {
    public:
        explicit WriteableMemMapFile(char const* pathname = 0,
            ExistsPolicyMMF exists_mode = ExistsPolicyMMF::if_exists_fail,
            DoesNotExistPolicyMMF doesnt_exist_mode = DoesNotExistPolicyMMF::if_doesnt_exist_create);
        WriteableMemMapFile(const std::string& path,
                ExistsPolicyMMF exists_mode = ExistsPolicyMMF::if_exists_fail,
                DoesNotExistPolicyMMF doesnt_exist_mode = DoesNotExistPolicyMMF::if_doesnt_exist_create):WriteableMemMapFile(path.c_str(),exists_mode,doesnt_exist_mode){
        }
        void open(char const* pathname,
            ExistsPolicyMMF exists_mode = ExistsPolicyMMF::if_exists_fail,
            DoesNotExistPolicyMMF doesnt_exist_mode = DoesNotExistPolicyMMF::if_doesnt_exist_create);
        inline void open(const std::string& path,
            ExistsPolicyMMF exists_mode = ExistsPolicyMMF::if_exists_fail,
            DoesNotExistPolicyMMF doesnt_exist_mode = DoesNotExistPolicyMMF::if_doesnt_exist_create){
        	open(path.c_str(), exists_mode, doesnt_exist_mode);
        }
        char* data() { return data_; }
        void map(size_t offset = 0, size_t size = 0);
        bool flush();
    };
}
#endif // MEMORY_MAPPED_FILE_HPP
