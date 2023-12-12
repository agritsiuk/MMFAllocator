#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <system_error>

extern "C" {
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
}

/**
 * @brief Memory-mapped file-based allocator
 * https://github.com/agritsiuk/MMFAllocator
 */
class MMFAllocator {
    using DataPtr = std::byte*;

  public:
    MMFAllocator() = default;
    MMFAllocator(std::filesystem::path filePath, std::size_t maxSize);

    virtual ~MMFAllocator();

    MMFAllocator(const MMFAllocator& other) = delete;
    MMFAllocator& operator=(const MMFAllocator& other) = delete;

    MMFAllocator(MMFAllocator&& other);
    MMFAllocator& operator=(MMFAllocator&& other);

    explicit operator bool() const;

    void open();
    DataPtr allocate(std::size_t size);
    void close();
    void release();

  private:
    void allocateBlock();
    static size_t alignCeil(std::size_t size, std::size_t alignMask);
    void removeFileIfExists();

  private:
    static constexpr std::size_t kPtrAlignmentMask = sizeof(DataPtr) - 1;
    static constexpr std::size_t kPageAlignmentMask = 4096 - 1;

  private:
    std::filesystem::path _filePath{};
    std::size_t _maxSize{};

    std::int32_t _fd{-1};
    std::size_t _size{};
    DataPtr _ptr{};
};

inline MMFAllocator::MMFAllocator(std::filesystem::path filePath, std::size_t maxSize)
    : _filePath{std::filesystem::absolute(filePath)}, _maxSize{alignCeil(maxSize, kPageAlignmentMask)} {}

inline MMFAllocator::MMFAllocator(MMFAllocator&& other) {
    *this = std::move(other);
}

inline MMFAllocator& MMFAllocator::operator=(MMFAllocator&& other) {
    if (this != &other) {
        close();

        _filePath = other._filePath;
        _maxSize = other._maxSize;

        _fd = other._fd;
        _size = other._size;

        other.release();
    }

    return *this;
}

inline MMFAllocator::operator bool() const {
    return _fd != -1;
}

inline MMFAllocator::~MMFAllocator() {
    close();
}

inline void MMFAllocator::open() {
    if (*this) {
        return;
    }

    removeFileIfExists();
    _fd = ::open(_filePath.c_str(), (O_CREAT | O_RDWR | O_EXCL), (S_IRUSR | S_IWUSR));

    if (_fd == -1) {
        throw std::system_error(errno, std::generic_category(), _filePath);
    }

    allocateBlock();
}

inline std::size_t MMFAllocator::alignCeil(std::size_t size, std::size_t alignMask) {
    return (size + kPtrAlignmentMask) & ~(kPtrAlignmentMask);
}

inline MMFAllocator::DataPtr MMFAllocator::allocate(std::size_t size) {
    std::size_t sizeAligned = alignCeil(size, kPtrAlignmentMask);

    if (_size + sizeAligned > _maxSize) {
        throw std::bad_alloc();
    }

    auto* ptr = _ptr + _size;
    _size += sizeAligned;
    return ptr;
}

inline void MMFAllocator::allocateBlock() {
    if (::ftruncate(_fd, _maxSize) == -1) {
        throw std::system_error(errno, std::generic_category(), _filePath);
    }

    auto ptr = static_cast<DataPtr>(::mmap(NULL, _maxSize, (PROT_READ | PROT_WRITE), MAP_SHARED, _fd, 0));

    if (ptr == MAP_FAILED) {
        throw std::system_error(ENOMEM, std::generic_category(), _filePath);
    }

    _ptr = ptr;
}

inline void MMFAllocator::close() {
    if (!*this) {
        return;
    }

    if (_ptr) {
        ::munmap(_ptr, _maxSize);
    }

    ::close(_fd);

    release();
    removeFileIfExists();
}

inline void MMFAllocator::release() {
    _fd = -1;
}

inline void MMFAllocator::removeFileIfExists() {
    std::error_code ec;
    std::filesystem::remove(_filePath, ec);
}