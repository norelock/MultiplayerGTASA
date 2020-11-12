/*
* Alpha Entity List Adjuster
* Copyright (c) 2014 ThirteenAG <thirteenag@gmail.com>
* Copyright (c) 2014 Silent <zdanio95@gmail.com>
* Copyright (c) 2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under the MIT License (http://opensource.org/licenses/MIT)
*/
#include "LimitAdjuster.h"
#include "utility/LinkListAdjuster.hpp"
#include "utility/dummy_object.hpp"

typedef dummy_object<0x8> AlphaObjectInfoVC;
typedef dummy_object<0xC> AlphaObjectInfoSA;

class AlphaEntityListSA : public LinkListAdjuster<AlphaObjectInfoSA>
{
    public:
        AlphaEntityListSA()
            : LinkListAdjuster(injector::lazy_ptr<0xC88120>().get())
        {}

        const char* GetLimitName()
        {
            return "AlphaEntityList";
        }

        void ChangeLimit(int, const std::string& value)
	    {
            if(Adjuster::IsUnlimited(value))
            {
                AddInsertSortedPatch(0x733DF5);
                AddInsertSortedPatch(0x7345F2);
            }
            else
            {
                // initializer is inlined in in GTA SA, needs to multiply by sizeof(CLinkList<AlphaObjectInfo>::Link)
                auto size = std::stoi(value) * 20; 
	            injector::WriteMemory(0x733B05, size, true);
	            injector::WriteMemory(0x733B55, size, true);
            }
	    }

} AlphaEntityListSA;
