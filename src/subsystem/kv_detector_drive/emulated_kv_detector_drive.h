#ifndef EMULATED_KV_DETECTOR_DRIVE_H
#define EMULATED_KV_DETECTOR_DRIVE_H

#include "kv_detector_drive.h"

class EmulatedKVDetectorDrive : public KVDetectorDrive
{
public:
    bool IsEmulated() override;
    void ModifyTelemetry() override;

};

#endif