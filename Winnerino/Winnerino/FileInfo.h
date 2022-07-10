#pragma once

namespace Winnerino::Storage
{
    class FileInfo
    {
    public:
        FileInfo(WIN32_FIND_DATA* findData, winrt::hstring path);

        /// <summary>
        /// Checks if the file is a directory.
        /// </summary>
        /// <returns>true if the file is a directory</returns>
        bool IsDirectory()
        {
            return attributes & FILE_ATTRIBUTE_DIRECTORY;
        };
        /// <summary>
        /// File name.
        /// </summary>
        /// <returns>Name of the file</returns>
        winrt::hstring FileName()
        {
            return fileName;
        };
        /// <summary>
        /// File path.
        /// </summary>
        /// <returns>Path of this life</returns>
        winrt::hstring FilePath()
        {
            return filePath;
        };
        /// <summary>
        /// Directory where the file is located.
        /// </summary>
        /// <returns>File's parent path</returns>
        winrt::hstring ParentPath()
        {
            return parentPath;
        };
        /// <summary>
        /// Attributes of the file.
        /// </summary>
        /// <returns>A mask of attributes</returns>
        DWORD Attributes()
        {
            return attributes;
        };
        /// <summary>
        /// Size of the file.
        /// </summary>
        /// <returns>Size of the file, in bytes</returns>
        uint64_t Size()
        {
            return size;
        };

    private:
        winrt::hstring fileName;
        winrt::hstring filePath;
        winrt::hstring parentPath;
        DWORD attributes;
        uint64_t size;
    };
}

