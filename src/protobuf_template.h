// Copyright © 2026 CCP ehf.

#pragma once

#include "pdm.h"
#include <google/protobuf/util/time_util.h>
#include "semver.h"

#if PDM_PROTO_USE_EVE_PUBLIC_DOMAIN

#include "eve_public/app/platform.pb.h"
#define PDM_PROTO_GET_DATA_NAME GetEVEPublicData
#define PDM_PROTO_BITNESS_UNSPECIFIED BITNESS_UNSPECIFIED
#define PDM_PROTO_OS_KIND_UNSPECIFIED KIND_UNSPECIFIED
#define PDM_PROTO_OS_KIND(KIND) KIND_ ## KIND

namespace platform = eve_public::app::platform;

#else

#include "eve_launcher/pdm.pb.h"
#define PDM_PROTO_GET_DATA_NAME GetEVELauncherData
#define PDM_PROTO_BITNESS_UNSPECIFIED BITNESS_UNKNOWN
#define PDM_PROTO_OS_KIND_UNSPECIFIED UNKNOWN
#define PDM_PROTO_OS_KIND(KIND) KIND

#endif

namespace
{
	platform::Bitness BitnessToProto(PDM::Bitness bitness)
	{
		switch(bitness)
		{
		case PDM::Bitness::BITNESS_32:
			return platform::BITNESS_32;
		case PDM::Bitness::BITNESS_64:
			return platform::BITNESS_64;
		case PDM::Bitness::BITNESS_UNKNOWN:
		default:
			return platform::PDM_PROTO_BITNESS_UNSPECIFIED;
		}
	}

	platform::Machine::CPU::Architecture CPUArchitectureToProto(PDM::CPUArchitecture archictecture)
	{
		switch(archictecture)
		{
		case PDM::CPUArchitecture::X86:
			return platform::Machine::CPU::ARCHITECTURE_X86;
		case PDM::CPUArchitecture::X86_64:
			return platform::Machine::CPU::ARCHITECTURE_X86_64;
		case PDM::CPUArchitecture::ARM:
			return platform::Machine::CPU::ARCHITECTURE_ARM;
		case PDM::CPUArchitecture::ARM64:
			return platform::Machine::CPU::ARCHITECTURE_ARM64;
		case PDM::CPUArchitecture::UNKNOWN:
			return platform::Machine::CPU::ARCHITECTURE_UNSPECIFIED;
		default:
			throw std::invalid_argument("Invalid CPU architecture");
		}
	}

	platform::OS::Kind OSKindToProto(PDM::OS osKind)
	{
		switch(osKind)
		{
		case PDM::OS::UNKNOWN:
			return platform::OS::PDM_PROTO_OS_KIND_UNSPECIFIED;
		case PDM::OS::WINDOWS:
			return platform::OS::PDM_PROTO_OS_KIND(WINDOWS);
		case PDM::OS::MACOS:
			return platform::OS::PDM_PROTO_OS_KIND(MACOS);
		case PDM::OS::WINE:
			return platform::OS::PDM_PROTO_OS_KIND(WINE);
		case PDM::OS::LINUX:
			return platform::OS::PDM_PROTO_OS_KIND(LINUX);
		default:
			throw std::invalid_argument("Invalid osType");
		}
	}

	platform::OS::StreamingService::Provider StreamingServiceToProto(PDM::StreamingService service)
	{
		switch(service)
		{
		case PDM::StreamingService::NONE:
			return platform::OS::StreamingService::PROVIDER_UNSPECIFIED;
		case PDM::StreamingService::UNKNOWN:
			return platform::OS::StreamingService::PROVIDER_UNKNOWN;
		case PDM::StreamingService::INTEL_STREAM:
			return platform::OS::StreamingService::PROVIDER_INTEL;
		default:
			throw std::invalid_argument("Invalid streaming service type");
		}
	}

	platform::Machine::BatteryDetection BatteryDetectionToProto(PDM::BatteryStatus status)
	{
		switch(status)
		{
		case PDM::BatteryStatus::UNKNOWN:
			return platform::Machine::BATTERY_UNSPECIFIED;
		case PDM::BatteryStatus::DETECTED:
			return platform::Machine::BATTERY_DETECTED;
		case PDM::BatteryStatus::NOT_DETECTED:
			return platform::Machine::BATTERY_NOT_DETECTED;
		default:
			throw std::invalid_argument("Invalid battery status");
		}
	}

	platform::Machine::HardDrive::DriveType HardDriveTypeToProto(PDM::HardDriveInfo::HardDriveType type)
	{
		switch (type)
		{
		case PDM::HardDriveInfo::HardDriveType::UNKNOWN:
			return platform::Machine::HardDrive::DRIVETYPE_UNSPECIFIED;
		case PDM::HardDriveInfo::HardDriveType::SSD:
			return platform::Machine::HardDrive::DRIVETYPE_SSD;
		case PDM::HardDriveInfo::HardDriveType::HDD:
			return platform::Machine::HardDrive::DRIVETYPE_HDD;
		default:
			throw std::invalid_argument("Invalid hard drive type");
		}
	}

	void AugmentVersion(const std::string& pdm_version, platform::SemanticVersion* result)
	{
		pdm::SemanticVersion version;

		(void)pdm::ParseSemanticVersion(pdm_version, version);

		result->set_major(version.major);
		result->set_minor(version.minor);
		result->set_patch(version.patch);
		result->set_prerelease(version.pre_release);
		result->set_build(version.build);
	}
}

namespace pdm_proto
{
	bool PDM_PROTO_GET_DATA_NAME(std::ostream* out)
	{
		platform::Information data;

        *data.mutable_timestamp() = google::protobuf::util::TimeUtil::GetCurrentTime();
        data.set_process_bitness(BitnessToProto(PDM::GetProcessBitness())); // Deprecated

        AugmentVersion(PDM::GetPDMVersion(), data.mutable_version());

		auto process = data.mutable_process();
		process->set_bitness(BitnessToProto(PDM::GetProcessBitness()));
		process->set_running_under_rosetta(PDM::IsRosetta());

		auto os = data.mutable_os();
		os->set_type(OSKindToProto(PDM::GetOSType()));
		os->set_name(PDM::GetOSName());
		os->set_bitness(BitnessToProto(PDM::GetOSBitness()));
		os->set_major_version(PDM::GetOSMajorVersion());
		os->set_minor_version(PDM::GetOSMinorVersion());
		os->set_build_number(PDM::GetOSBuildNumber());
		os->set_kernel_version(PDM::GetOSKernelVersion());
		os->set_username(PDM::GetUsername());
		os->set_user_locale(PDM::GetUserLocale());
		os->set_is_remote_session(PDM::IsRemoteSession());
		
		auto streamingService = os->mutable_streaming_service();
		streamingService->set_provider(StreamingServiceToProto(PDM::GetStreamingService()));

		auto graphicsAPIs = os->mutable_graphics_apis();
		graphicsAPIs->set_metal_supported(PDM::GetMetalSupported());
		auto vulkan = PDM::GetVulkanProperties();
		graphicsAPIs->set_vulkan_supported(vulkan.support == PDM::VulkanSupport::SUPPORTED);
		graphicsAPIs->set_vulkan_highest_supported_version(vulkan.version);
		graphicsAPIs->set_d3d_highest_supported_version(PDM::GetD3DHighestSupport());

		auto wine = os->mutable_wine();
		wine->set_version(PDM::GetWineVersion());
		wine->set_host_os(PDM::GetWineHostOs());

		auto machine = data.mutable_machine();
		machine->set_name(PDM::GetMachineName());
		machine->set_model(PDM::GetHardwareModel());
		auto uuid = PDM::GetMachineUuid();
		machine->set_uuid(uuid.data(), uuid.size());
		machine->set_total_memory(PDM::GetTotalMemory());
		machine->set_monitor_count(PDM::GetMonitorCount());
		machine->set_battery_detection(BatteryDetectionToProto(PDM::GetBatteryStatus()));
	
		auto cpu = machine->mutable_cpu();
		auto cpuInfo = PDM::GetCPUInfo();
		cpu->set_bitness(BitnessToProto(cpuInfo.bitness));
		cpu->set_logical_core_count(cpuInfo.logicalCoreCount);
		cpu->set_brand(cpuInfo.brand);
		cpu->set_vendor(cpuInfo.vendor);
		cpu->set_model(cpuInfo.model);
		cpu->set_stepping(cpuInfo.stepping);
		cpu->set_architecture(CPUArchitectureToProto(cpuInfo.architecture));
		cpu->set_frequency(cpuInfo.frequency);
		for (const auto& extension : cpuInfo.extensions)
			cpu->add_extensions(extension);

		auto vm = machine->mutable_vm();
		vm->set_is_suspected_vm(PDM::IsSuspectedVM());
		vm->set_has_hypervisor_bit(PDM::HasHypervisorBit());
		vm->set_hypervisor_name(PDM::GetHypervisorName());
		vm->set_is_hypervisor_guest_os(PDM::IsHyperVGuestOS());
		vm->set_has_vm_execution_timing(PDM::HasVMExecutionTiming());

		for(const auto& monitorData : PDM::GetMonitorsInfo())
		{
			auto monitor = machine->add_monitors();
			monitor->set_name(monitorData.name);
			monitor->set_horizontal_resolution(monitorData.width);
			monitor->set_vertical_resolution(monitorData.height);
			monitor->set_bits_per_color(monitorData.bitsPerColor);
			monitor->set_refresh_rate(monitorData.refreshRate);
			monitor->set_dpi_scaling_percent(monitorData.dpiScaling);
		}

		for(const auto& gpuData : PDM::GetGPUInfo())
		{
			auto gpu = machine->add_gpus();
			gpu->set_description(gpuData.description);
			gpu->set_vendor_id(gpuData.vendorID);
			gpu->set_device_id(gpuData.deviceID);
			gpu->set_revision(gpuData.revision);
			gpu->set_video_memory(gpuData.memory);
			gpu->set_core_count(gpuData.coreCount);

			auto driver = gpu->mutable_driver();
			driver->set_date(gpuData.driverDate);
			driver->set_vendor(gpuData.driverVendor);
			driver->set_version(gpuData.driverVersionString);
		}

		for(const auto& networkAdapterData : PDM::GetNetworkAdapterInfo())
		{
			auto networkAdapter = machine->add_network_adapters();
			networkAdapter->set_name(networkAdapterData.name);
			networkAdapter->set_mac_address(networkAdapterData.macAddress.data(), networkAdapterData.macAddress.size());
			networkAdapter->set_uuid(networkAdapterData.uuid.data(), networkAdapterData.uuid.size());
		}

		for (const auto& hardDriveData : PDM::GetHardDriveInfo())
		{
			auto hardDrive = machine->add_hard_drives();
			hardDrive->set_name(hardDriveData.name);
			hardDrive->set_drive_type(HardDriveTypeToProto(hardDriveData.type));
			hardDrive->set_size(hardDriveData.size);
		}

		return data.SerializeToOstream(out);
	}

#if PDM_PROTO_USE_EVE_PUBLIC_DOMAIN
auto GetData = PDM_PROTO_GET_DATA_NAME;
#endif
}
