/*
* TimeModelInfo Adjuster for GTA 3/VC/SA
* Copyright (c) 2014 ThirteenAG <thirteenag@gmail.com>
* Licensed under the MIT License (http://opensource.org/licenses/MIT)
*/
#include "StoreAdjuster.hpp"
#include "utility/dummy_object.hpp"

typedef dummy_object_vmt<0x24, 0x4C5640> CTimeModelInfo_SA;

struct TimeModelInfoSA : public StoreAdjuster<CTimeModelInfo_SA, 0xB1C960, 169>    // T, pDefaultStore, dwDefaultCapacity
{
    const char* GetLimitName()
    {
        return "TimeModels";
    }

    TimeModelInfoSA()
    {
        this->SetGrower (0x5B3F32);
        this->AddPointer(0x4C6459, 0x0);
        this->AddPointer(0x4C6470, 0x0);
        this->AddPointer(0x4C65EC, 0x0);
        this->AddPointer(0x4C66B1, 0x0);
        this->AddPointer(0x4C66C2, 0x0);
        this->AddPointer(0x4C683B, 0x0);
        //this->AddPointer(0x84BC51, 0x0);
        //this->AddPointer(0x856261, 0x0);
        this->AddPointer(0x4C6464, 0x4);
        this->AddPointer(0x4C66BD, 0x4);
    }

} TimeModelInfoSA;