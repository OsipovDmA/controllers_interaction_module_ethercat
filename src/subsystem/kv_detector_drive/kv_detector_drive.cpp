#include "kv_detector_drive.h"
#include "debug_logger.h"
#include "coe_object_names.h"
#include "ethercat_slave_names.h"

KVDetectorDrive::KVDetectorDrive(uint32_t microstep_resolution, float thread_pitch)
{
    this->props.microstep_resolution = microstep_resolution;
    this->props.thread_pitch = thread_pitch;

    //logger.CreateStorage(4, 26 * 1000, "measure/kv_detector_drive_5mms_500al.txt");
}

KVDetectorDrive::~KVDetectorDrive()
{
    delete wire_sensor;
}


void KVDetectorDrive::Action()
{
    auto telemetry = this->context->GetTelemetryExchanger()->GetMasterTelemetry();
    /*logger.PlaceData(
        {
            telemetry->drive_position_mm,
            telemetry->drive_velocity_mm_per_sec,
            telemetry->wire_sensor_position_mm,
            telemetry->wire_sensor_velocity_mm_per_sec
        }
    ); */
    //std::cout << this->GetEthercatConfig()->GetSlave(ethercat_slave_names::kIOModuleName)->
    //GetTxPDOEntry(coe_object_names::kCT3168_AI0)->LoadValue() << std::endl;
    //std::cout << telemetry->wire_sensor_position_mm << std::endl;
    if(this->wire_sensor) wire_sensor->Update();
    SubsystemContentCreator<KVDetectorDriveContext, KVDetectorDriveTask, KVDetectorDrive, KVDetectorDriveTelemetry>::Action(); 
}

KVDetectorDrive::Properties KVDetectorDrive::GetProperties()
{
    return this->props;
}

void KVDetectorDrive::RegisterWireSensor(WireSensor* sensor)
{
    this->wire_sensor = sensor;
}

void KVDetectorDrive::RegisterDrive(EthercatSlave* drive)
{
    this->drive = drive;
}

EthercatSlave* KVDetectorDrive::GetDrive()
{
    return this->drive;
}