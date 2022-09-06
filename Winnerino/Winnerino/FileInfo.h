#pragma once

namespace Winnerino::Storage
{
    class FileInfo
    {
    public:
        FileInfo(WIN32_FIND_DATA* findData, winrt::hstring parentPath);
        ~FileInfo();

        /// <summary>
        /// Attributes of the file.
        /// </summary>
        /// <returns>A mask of attributes</returns>
        DWORD Attributes() const;
        /// <summary>
        /// Checks if the file is a directory.
        /// </summary>
        /// <returns>true if the file is a directory</returns>
        bool IsDirectory() const;
        /// <summary>
        /// Checks if the file is a system file.
        /// </summary>
        /// <returns>true if the file has the system file attribute</returns>
        bool IsSystemFile() const;
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        winrt::Windows::Foundation::DateTime LastWrite() const;
        /// <summary>
        /// File name.
        /// </summary>
        /// <returns>Name of the file</returns>
        winrt::hstring Name() const;
        /// <summary>
        /// File path.
        /// </summary>
        /// <returns>Path of this life</returns>
        winrt::hstring Path() const;
        /// <summary>
        /// Directory where the file is located.
        /// </summary>
        /// <returns>File's parent path</returns>
        winrt::hstring ParentPath() const;
        /// <summary>
        /// Size of the file.
        /// </summary>
        /// <returns>Size of the file, in bytes</returns>
        uint64_t Size() const;

    private:
        winrt::hstring fileName;
        winrt::hstring filePath;
        winrt::hstring parentPath;
        DWORD attributes;
        uint64_t size;
        winrt::Windows::Foundation::DateTime lastWrite;
    };
}

