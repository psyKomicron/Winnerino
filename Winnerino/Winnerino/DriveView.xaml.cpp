#include "pch.h"
#include "fileapi.h"
#include "ioapiset.h"
#include "nvme.h"
#include "winioctl.h"
#include <chrono>
#include "DriveView.xaml.h"
#if __has_include("DriveView.g.cpp")
#include "DriveView.g.cpp"
#endif
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    DriveView::DriveView()
    {
        _driveName = L"";
        _driveFreeBytes = 0;
        _driveSize = 0;
        _fileSystemType = L"";
        _maxFileNameLength = 0;
        _volumeSerialNumber = 0;
        _driveType = L"";
        InitializeComponent();
    }

    DriveView::DriveView(winrt::hstring const& driveName)
    {
        _driveName = driveName;

        WCHAR friendlyName[MAX_PATH + 1];
        DWORD volumeSerialNumber = 0;
        DWORD volumeMaxComponentLength = 0;
        DWORD systemFlags = 0;
        WCHAR fileSystemName[MAX_PATH];
        if (GetVolumeInformation(driveName.c_str(), friendlyName, MAX_PATH + 1, &volumeSerialNumber, &volumeMaxComponentLength, &systemFlags, fileSystemName, MAX_PATH))
        {
            _volumeSerialNumber = volumeSerialNumber;
            _maxFileNameLength = volumeMaxComponentLength;
            _fileSystemType = fileSystemName;

            switch (GetDriveType(driveName.c_str()))
            {
                case DRIVE_REMOVABLE:
                    _driveType = L"Removable drive";
                    break;
                case DRIVE_FIXED:
                    _driveType = L"Hard drive";
                    break;
                case DRIVE_REMOTE:
                    _driveType = L"Remote drive";
                    break;
                case DRIVE_CDROM:
                    _driveType = L"CD";
                    break;
                case DRIVE_RAMDISK:
                    _driveType = L"Ram disk";
                    break;
                default:
                    _driveType = L"Unknow drive type";
                    break;
            }

            // no initialization since it will be done by GetDiskFreeSpace
            ULARGE_INTEGER totalNumbersOfFreeBytes, totalNumberOfBytes;
            if (GetDiskFreeSpaceEx(driveName.c_str(), NULL, &totalNumberOfBytes, &totalNumbersOfFreeBytes))
            {
                _driveFreeBytes = (static_cast<int64_t>(totalNumbersOfFreeBytes.HighPart) << 32) | totalNumbersOfFreeBytes.LowPart;
                _driveSize = (static_cast<int64_t>(totalNumberOfBytes.HighPart) << 32) | totalNumberOfBytes.LowPart;
            }
            InitializeComponent();
        }
        else
        {
            InitializeComponent();
            MainWindow::Current().NotifyError(GetLastError());
        }
    }

    void DriveView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        double ratio = (static_cast<double>(_driveSize - _driveFreeBytes) / _driveSize);
        double width = (ActualWidth() - Padding().Right) * ratio;

#if TRUE
        std::string fileName = to_string(_driveName);
        fileName = "\\\\.\\" + fileName.substr(0, fileName.size() - 1);

        // only works with elevated privileges
        HANDLE diskHandle = CreateFile(to_hstring(fileName).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (diskHandle != INVALID_HANDLE_VALUE)
        {
            // retreive disk capacity (in bytes)
            GET_LENGTH_INFORMATION lengthInfo{ 0 };
            if (DeviceIoControl(diskHandle, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &lengthInfo, sizeof(lengthInfo), NULL, NULL))
            {
                _driveSize = (static_cast<int64_t>(lengthInfo.Length.HighPart) << 32) | lengthInfo.Length.LowPart;
                width = ActualWidth() * ((static_cast<double>(_driveSize) - _driveFreeBytes) / _driveSize);
            }
            else
            {
                DWORD lastError = GetLastError();
                if (lastError == ERROR_ACCESS_DENIED)
                {
                    MainWindow::Current().NotifyError(lastError, L"Failed to get " + _driveName + L" drive size. Try to run with administator privileges");
                }
                else
                {
                    MainWindow::Current().NotifyError(lastError, L"Failed to get drive information");
                }
            }

            if (!LoadForNVME(diskHandle))
            {
                LoadForATA(diskHandle);
            }
            // close handle
            CloseHandle(diskHandle);
        }
        else
        {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_ACCESS_DENIED)
            {
                MainWindow::Current().NotifyError(lastError, L"Failed to get " + _driveName + L" drive information. Try to run with administator privileges");
            }
            else
            {
                MainWindow::Current().NotifyError(lastError, L"Failed to get drive handle");
            }
        }
#endif // Get detailed drive informations

        ratioDisplay().Width(width);
    }

    bool DriveView::LoadForNVME(HANDLE const& diskHandle)
    {
        DWORD bufferLength = FIELD_OFFSET(STORAGE_PROPERTY_QUERY, AdditionalParameters) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
        PVOID buffer = malloc(bufferLength);
        if (buffer)
        {
            ZeroMemory(buffer, bufferLength);

            PSTORAGE_PROPERTY_QUERY query = (PSTORAGE_PROPERTY_QUERY)buffer;
            query->PropertyId = STORAGE_PROPERTY_ID::StorageAdapterProtocolSpecificProperty;
            query->QueryType = STORAGE_QUERY_TYPE::PropertyStandardQuery;

            PSTORAGE_PROTOCOL_DATA_DESCRIPTOR protocolDataDesc = (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)buffer;

            PSTORAGE_PROTOCOL_SPECIFIC_DATA protocolData = (PSTORAGE_PROTOCOL_SPECIFIC_DATA)query->AdditionalParameters;
            protocolData->ProtocolType = STORAGE_PROTOCOL_TYPE::ProtocolTypeNvme;
            protocolData->DataType = STORAGE_PROTOCOL_NVME_DATA_TYPE::NVMeDataTypeIdentify;
            protocolData->ProtocolDataRequestValue = NVME_IDENTIFY_CNS_CODES::NVME_IDENTIFY_CNS_CONTROLLER;
            protocolData->ProtocolDataRequestSubValue = 0;
            protocolData->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
            protocolData->ProtocolDataLength = NVME_MAX_LOG_SIZE;

            DWORD returnedLength = 0;
            // Will fail is the device is not a NVME drive
            if (DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, buffer, bufferLength, buffer, bufferLength, &returnedLength, NULL) && returnedLength > 0)
            {
                MainWindow::Current().NotifyUser(L"Retreiving NVME information for drive " + _driveName, InfoBarSeverity::Informational);

                if (protocolDataDesc->Size == sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR) &&
                    protocolDataDesc->Version == sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR))
                {
                    protocolData = &protocolDataDesc->ProtocolSpecificData;
                    if (protocolData->ProtocolDataOffset == sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) &&
                        protocolData->ProtocolDataLength == NVME_MAX_LOG_SIZE)
                    {
                        //NVME_IDENTIFY_CONTROLLER_DATA
                        PNVME_IDENTIFY_CONTROLLER_DATA controllerData = (PNVME_IDENTIFY_CONTROLLER_DATA)((PCHAR)protocolData + protocolData->ProtocolDataOffset);
                        if (controllerData && controllerData->VID != 0 && controllerData->NN != 0)
                        {
                            UIElementCollection const& infoPanel = nvmeInfoPanel().Children();
                            // Show informations
                            infoPanel.Append(CreateTextBlock(L"PCI Vendor ID: " + to_hstring(controllerData->VID)));
                            infoPanel.Append(CreateTextBlock(L"PCI Subsystem Vendor ID: " + to_hstring(controllerData->SSVID)));
                            infoPanel.Append(CreateTextBlock(L"Serial number: " + ConvertToHString(controllerData->SN, ARRAYSIZE(controllerData->SN))));
                            infoPanel.Append(CreateTextBlock(L"Model number: " + ConvertToHString(controllerData->MN, ARRAYSIZE(controllerData->MN))));
                            infoPanel.Append(CreateTextBlock(L"Firmware revision: " + ConvertToHString(controllerData->FR, ARRAYSIZE(controllerData->FR))));
                            infoPanel.Append(CreateTextBlock(L"Recommended Arbitration Burst: " + to_hstring(controllerData->RAB)));
                            infoPanel.Append(CreateTextBlock(L"Resume latency: " + to_hstring((uint32_t)controllerData->RTD3R)));
                            infoPanel.Append(CreateTextBlock(L"Entry latency: " + to_hstring((uint32_t)controllerData->RTD3E)));
                            // power states
                        }
                    }
                }
                else
                {
                    MainWindow::Current().NotifyUser(_driveName + L" - Data descriptor header not valid.", InfoBarSeverity::Error);
                }

                // Query for health informations
                // Reset structure
                ZeroMemory(buffer, bufferLength);
                query = (PSTORAGE_PROPERTY_QUERY)buffer;
                protocolDataDesc = (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)buffer;
                protocolData = (PSTORAGE_PROTOCOL_SPECIFIC_DATA)query->AdditionalParameters;

                query->PropertyId = StorageDeviceProtocolSpecificProperty;
                query->QueryType = PropertyStandardQuery;

                protocolData->ProtocolType = STORAGE_PROTOCOL_TYPE::ProtocolTypeNvme;
                protocolData->DataType = STORAGE_PROTOCOL_NVME_DATA_TYPE::NVMeDataTypeLogPage;
                protocolData->ProtocolDataRequestValue = NVME_LOG_PAGES::NVME_LOG_PAGE_HEALTH_INFO;
                protocolData->ProtocolDataRequestSubValue = 0;
                protocolData->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
                protocolData->ProtocolDataLength = sizeof(NVME_HEALTH_INFO_LOG);

                if (DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, buffer, bufferLength, buffer, bufferLength, &returnedLength, NULL))
                {
                    MainWindow::Current().NotifyUser(L"Retreiving NVME smart information for drive " + _driveName, InfoBarSeverity::Informational);

                    if (protocolDataDesc->Size == sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR) &&
                        protocolDataDesc->Version == sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR))
                    {
                        protocolData = &protocolDataDesc->ProtocolSpecificData;
                        if (protocolData->ProtocolDataOffset == sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) &&
                            protocolData->ProtocolDataLength == sizeof(NVME_HEALTH_INFO_LOG))
                        {
                            PNVME_HEALTH_INFO_LOG smartInfo = (PNVME_HEALTH_INFO_LOG)((PCHAR)protocolData + protocolData->ProtocolDataOffset);
                            if (smartInfo)
                            {
                                UIElementCollection const& panel = nvmeSmartInfoPanel().Children();
                                uint16_t diskTemperature = (static_cast<uint16_t>(smartInfo->Temperature[1]) << 8 | smartInfo->Temperature[0]) - 273;
                                panel.Append(CreateTextBlock(L"Temperature: " + to_hstring(diskTemperature)));

                                uint64_t powerOnHours = (static_cast<uint64_t>(smartInfo->PowerOnHours[7]) << 56) |
                                    (static_cast<uint64_t>(smartInfo->PowerOnHours[6]) << 48) |
                                    (static_cast<uint64_t>(smartInfo->PowerOnHours[5]) << 40) |
                                    (static_cast<uint64_t>(smartInfo->PowerOnHours[4]) << 32) |
                                    (static_cast<uint64_t>(smartInfo->PowerOnHours[3]) << 24) |
                                    (static_cast<uint64_t>(smartInfo->PowerOnHours[2]) << 16) |
                                    (static_cast<uint64_t>(smartInfo->PowerOnHours[1]) << 8) |
                                    smartInfo->PowerOnHours[0];
                                panel.Append(CreateTextBlock(L"Power on hours: " + to_hstring(powerOnHours)));
                            }
                        }
                    }
                }
                else
                {
                    MainWindow::Current().NotifyError(GetLastError(), L"Failed to get NVME Smart Info");
                }
            }
            else
            {
                free(buffer);
                int32_t lastError = GetLastError();
                TextBlock textBlock = CreateTextBlock(L"Could not retreive NVMe data.");
                ::Style style = Application::Current().Resources().TryLookup(box_value(L"TitleTextBlockStyle")).try_as<::Style>();
                if (style)
                {
                    textBlock.Style(style);
                }
                nvmeInfoPanel().Children().Append(textBlock);
                return false;
            }

            free(buffer);
            return true;
        }
        
        return false;
    }

    bool DriveView::LoadForATA(HANDLE const& diskHandle)
    {
        DWORD bufferLength = FIELD_OFFSET(STORAGE_PROPERTY_QUERY, AdditionalParameters) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + NVME_MAX_LOG_SIZE;
        PVOID buffer = malloc(bufferLength);
        if (buffer)
        {
            ZeroMemory(buffer, bufferLength);

            PSTORAGE_PROPERTY_QUERY query = (PSTORAGE_PROPERTY_QUERY)buffer;
            query->PropertyId = STORAGE_PROPERTY_ID::StorageDeviceDeviceTelemetryProperty;
            query->QueryType = STORAGE_QUERY_TYPE::PropertyStandardQuery;

            PSTORAGE_PROTOCOL_DATA_DESCRIPTOR protocolDataDesc = (PSTORAGE_PROTOCOL_DATA_DESCRIPTOR)buffer;

            PSTORAGE_PROTOCOL_SPECIFIC_DATA protocolData = (PSTORAGE_PROTOCOL_SPECIFIC_DATA)query->AdditionalParameters;
            protocolData->ProtocolType = STORAGE_PROTOCOL_TYPE::ProtocolTypeAta;
            protocolData->DataType = STORAGE_PROTOCOL_ATA_DATA_TYPE::AtaDataTypeIdentify;
            protocolData->ProtocolDataRequestValue = 1;
            protocolData->ProtocolDataRequestSubValue = 0;
            protocolData->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
            protocolData->ProtocolDataLength = sizeof(STORAGE_PHYSICAL_ADAPTER_DATA);

            DWORD returnedLength = 0;
            // Will fail is the device is not a NVME drive
            if (DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, buffer, bufferLength, buffer, bufferLength, &returnedLength, NULL) && returnedLength > 0)
            {
                MainWindow::Current().NotifyUser(L"Retreiving ATA information for drive " + _driveName, InfoBarSeverity::Informational);

                if (protocolDataDesc->Size == sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR) &&
                    protocolDataDesc->Version == sizeof(STORAGE_PROTOCOL_DATA_DESCRIPTOR))
                {
                    protocolData = &protocolDataDesc->ProtocolSpecificData;
                    if (protocolData->ProtocolDataOffset == sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) &&
                        protocolData->ProtocolDataLength == NVME_MAX_LOG_SIZE)
                    {
                        //NVME_IDENTIFY_CONTROLLER_DATA
                        PNVME_IDENTIFY_CONTROLLER_DATA controllerData = (PNVME_IDENTIFY_CONTROLLER_DATA)((PCHAR)protocolData + protocolData->ProtocolDataOffset);
                        
                    }
                }
                else
                {
                    MainWindow::Current().NotifyUser(_driveName + L" - Data descriptor header not valid.", InfoBarSeverity::Error);
                }
            }
            else
            {
                free(buffer);
                MainWindow::Current().NotifyError(GetLastError(), L"(" + _driveName + L")" L"While retreiving ATA information:");
                TextBlock textBlock = CreateTextBlock(L"Failed to get ATA information.");
                ::Style style = Application::Current().Resources().TryLookup(box_value(L"TitleTextBlockStyle")).try_as<::Style>();
                if (style)
                {
                    textBlock.Style(style);
                }
                nvmeSmartInfoPanel().Children().Append(textBlock);
                return false;
            }

            free(buffer);
            return true;
        }

        return false;
    }

    inline winrt::Microsoft::UI::Xaml::Controls::TextBlock DriveView::CreateTextBlock(winrt::hstring const& text)
    {
        TextBlock textBlock = TextBlock{};
        textBlock.IsTextSelectionEnabled(true);
        textBlock.Text(text);
        return textBlock;
    }

    inline hstring DriveView::ConvertToHString(UCHAR* const& toConvert, size_t const& length)
    {
        char* original = (char*)toConvert;
        size_t convertedChars = 0;
        std::unique_ptr<wchar_t> wideString(new wchar_t[length + 1]);
        mbstowcs_s(&convertedChars, wideString.get(), length + 1, original, _TRUNCATE);
        hstring converted = to_hstring(wideString.get());
        return converted;
    }
}
