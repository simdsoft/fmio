//////////////////////////////////////////////////////////////////////////////////////////
// A cross platform file mapping io helper.
//////////////////////////////////////////////////////////////////////////////////////////
/*
The MIT License (MIT)

Copyright (c) 2019-2020 halx99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef SIMDSOFT__FMIO_HPP
#define SIMDSOFT__FMIO_HPP

#if defined(_WIN32)

class fmio
{
public:
    fmio(int fd, long size) : _Myhandle(nullptr), _Mybase(nullptr)
    {
        int mode = PAGE_READONLY;
        auto hFile = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
        if (hFile != nullptr) {
            _Myhandle = ::CreateFileMapping(hFile,
                nullptr,
                mode,
                0,
                size,
                nullptr);
        }
    }
    ~fmio()
    {
        if (is_open())
        {
            unmap();
            ::CloseHandle(_Myhandle);
            _Myhandle = nullptr;
        }
    }

    bool is_open() const
    {
        return _Myhandle != nullptr;
    }

    void* map(long offset, long size)
    {
        int mode = FILE_MAP_READ;
        _Mybase = ::MapViewOfFile(_Myhandle, mode, 0, offset, size);
        return _Mybase;
    }

    void unmap(void)
    {
        if (_Mybase != nullptr)
        {
            ::UnmapViewOfFile(_Mybase);
            _Mybase = nullptr;
        }
    }

    static long get_pagesize()
    {
        static long s_pagesize = -1;
        if (s_pagesize != -1)
            return s_pagesize;

        SYSTEM_INFO sysinfo = { 0 };
        ::GetSystemInfo(&sysinfo);
        s_pagesize = sysinfo.dwAllocationGranularity;
        return s_pagesize;
    }

private:
    HANDLE _Myhandle;
    void* _Mybase;
};

#else

class fmio
{
public:
    fmio(int fd, long /*size*/)
    {
        this->fd_ = fd;
        this->ptr_ = nullptr;
        this->size_ = -1;
    }
    ~fmio()
    {
        if (is_open())
        {
            unmap();
            this->fd_ = 0;
        }
    }

    bool is_open() const
    {
        return this->fd_ != 0;
    }

    void* map(long offset, long size)
    {
        this->size_ = size;
        this->ptr_ = ::mmap(NULL, this->size_, PROT_READ, MAP_PRIVATE, this->fd_, offset);
        return this->ptr_;
    }

    void unmap(void)
    {
        if (this->ptr_ != nullptr)
        {
            ::munmap(this->ptr_, this->size_);
            this->ptr_ = nullptr;
            this->size_ = -1;
        }
    }

    static long get_pagesize()
    {
        static long s_pagesize = -1;
        if (s_pagesize != -1)
            return s_pagesize;

        s_pagesize = sysconf(_SC_PAGESIZE);
        return s_pagesize;
    }

private:
    int fd_;

    void* ptr_;
    long size_;
};
#endif

#endif
