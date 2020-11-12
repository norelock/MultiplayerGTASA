/*
* Visible Entities List Adjuster
* Copyright (C) 2014 ThirteenAG <thirteenag@gmail.com>
* Copyright (C) 2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under the MIT License (http://opensource.org/licenses/MIT)
*/
#include "LimitAdjuster.h"
#include "utility/StaticArrayAdjuster.hpp"
#include <injector/assembly.hpp>

/*
    VisibilityPtrsBase
        Base for the visibility pointers rellocation
        @pDefaultArray is the address of the default array at the data segment
        @DefaultArraySize is the default size of that array
        @pArrayUsage is the address where the array usage is stored
*/
template<uintptr_t pDefaultArray, size_t DefaultArraySize, uintptr_t pArrayUsage>
struct VisibilityPtrsBase : public StaticArrayAdjuster<void*>
{
    private:
        size_t& m_dwUsage;                                      // Reference to the array usage (pArrayUsage)
        std::vector<injector::memory_pointer_raw> m_Growers;    // List of (mov eax, m_dwUsage) on the code, so we can check if rellocation is necessary
                                                                // (Yeah, the three games use this same register and everything)

        // Just to access it from the DoGrowFun functor, unique because of the template parameters
        static VisibilityPtrsBase*& Instance()
        {
             static VisibilityPtrsBase* myself;
             return myself;
        }

    public:

        VisibilityPtrsBase() :
            StaticArrayAdjuster(pDefaultArray, DefaultArraySize),
            m_dwUsage(*injector::lazy_ptr<pArrayUsage>().get<size_t>())
        {
            this->Instance() = this;
        }

        // Adds a grower address, that's a (mov eax, dwUsage) right before incrementing and adding a element to the array
        // The hook this will produce will be used to check if the array needs to grow
        void AddGrower(uintptr_t addr)
        {
            using namespace injector;
            m_Growers.push_back(raw_ptr(memory_pointer(addr)));
        }

        // Called when the limit is found on the ini
	    virtual void ChangeLimit(int, const std::string& value)
	    {
            if(Adjuster::IsUnlimited(value))
            {
                // Inline '''assembler''' functor for ensuring the capacity is fine
                struct DoGrowFun
                {
                    // All the growing procs for those visibility lists use eax as indexer :)
                    void operator()(injector::reg_pack& regs)
                    {
                        auto m_dwUsage = Instance()->m_dwUsage;
                        Instance()->EnsureHasCapacityFor(m_dwUsage + 1);
                        regs.eax = m_dwUsage;   // replaces a (mov eax, m_dwUsage)
                    }
                };

                // Makes the inline grow functor
                for(auto it = m_Growers.begin(); it != m_Growers.end(); ++it)
                    injector::MakeInline<DoGrowFun>(*it);
            }
            else
            {
                this->RellocArray(std::stoi(value));
            }
	    }

        // Gets the current usage of this visibility array
        virtual bool GetUsage(int, std::string& output)
        {
            return Adjuster::GetUsage(output, m_dwUsage, this->GetArraySize());
        }
};



//
//  The actual adjusters
//

struct VisibleEntityPtrsSA : VisibilityPtrsBase<0xB75898, 1000, 0xB76844>    // <pDefault, dwMax, dwUsage>
{
	const char* GetLimitName()
    {
        return "VisibleEntityPtrs";
    }
	
    VisibleEntityPtrsSA()
    {
        this->AddGrower (0x553521);
        this->AddPointer(0x553529);
        this->AddPointer(0x553944);
        this->AddPointer(0x553A53);
        this->AddPointer(0x553B03);
    }

} VisibleEntityPtrsSA;

struct VisibleLodPtrsSA : VisibilityPtrsBase<0xB748F8, 1000, 0xB76840>    // <pDefault, dwMax, dwUsage>
{
	const char* GetLimitName()
    {
        return "VisibleLodPtrs";
    }

    VisibleLodPtrsSA()
    {
        this->AddGrower (0x5534ED);
        this->AddPointer(0x5534F5);
        this->AddPointer(0x553923);
        this->AddPointer(0x553CB3);
    }

} VisibleLodPtrsSA;
