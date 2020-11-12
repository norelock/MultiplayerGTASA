/*
 *  FrameLimit Adjuster
 *
 */
#include "LimitAdjuster.h"

// injector includes windows.h without defining NOMINMAX...
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

static int ReadFrameLimitValue( const std::string& value )
{
    const int fpsValue = std::stoi(value);
    return fpsValue > 0 ? fpsValue : std::numeric_limits<int>::max();
}
 
class FrameLimitSA : public SimpleAdjuster
{
    public:
        const char* GetLimitName() { return "FrameLimit"; }
        void ChangeLimit(int, const std::string& value) 
        { 
            const int fpsValue = ReadFrameLimitValue( value );

            injector::WriteMemory(0x619626, fpsValue, true); 
            injector::WriteMemory(0xC1704C, fpsValue, false);
        }
        
} FrameLimitSA;