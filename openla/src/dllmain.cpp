/*
 *  Grand Theft Auto Limit Adjuster - Base Code
 *      Copyright (C) 2013-2014 ThirteenAG <thirteenag@gmail.com>
 *      Copyright (C) 2014 LINK/2012 <dma_2012@hotmail.com>
 *      Licensed under the MIT License (http://opensource.org/licenses/MIT)
 */
#include <windows.h>
#include <LimitAdjuster.h>
#include <ini_parser/ini_parser.hpp>
#include <injector/utility.hpp>
#include <injector/gvm/translator.hpp>

// Stores an handler for a specific limit
struct LimitHandler
{ 
    int         id;         // Id for the key
    Adjuster*   adjuster;   // Adjuster to handle the key
};

static int vkDebugText = VK_F5; // 0x74
int AdjustLimits();
void PatchDrawer();
void AdjustLimits(const std::map<std::string, std::string>& section);

// Map of name of limit and adjuster to handle that limit
static std::map<std::string, LimitHandler> limits;

// Entry point
extern "C"
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    if(reason == DLL_PROCESS_ATTACH)
    {        
        AdjustLimits();             // Run the limit adjuster
        PatchDrawer();
    }
    return TRUE;
}

/*
 * -------- Basic Limit Adjuster Running --------
 */

/*
 *  Adjust the limits asked by the ini file
 */
int AdjustLimits()
{
	std::map<std::string, std::string> keys;
	keys.insert(std::pair<std::string, std::string>("PtrNodeSingle", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("PtrNodeDouble", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("EntryInfoNode", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("Peds", "2000"));
	keys.insert(std::pair<std::string, std::string>("PedIntelligence", "2000"));
	keys.insert(std::pair<std::string, std::string>("Vehicles", "2000"));
	keys.insert(std::pair<std::string, std::string>("Buildings", "100000"));
	keys.insert(std::pair<std::string, std::string>("Objects", "10000"));
	keys.insert(std::pair<std::string, std::string>("Dummys", "50000"));
	keys.insert(std::pair<std::string, std::string>("ColModel", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("Task", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("Event", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("PointRoute", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("PatrolRoute", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("NodeRoute", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("TaskAllocator", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("PedAttractors", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("VehicleStructs", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("MatrixList", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("OutsideWorldWaterBlocks", "500"));
	keys.insert(std::pair<std::string, std::string>("AlphaEntityList", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("VisibleEntityPtrs", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("VisibleLodPtrs", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("StreamingObjectInstancesList", "30000"));
	keys.insert(std::pair<std::string, std::string>("AtomicModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("DamageAtomicModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("TimeModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("ClumpModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("VehicleModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("PedModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("WeaponModels", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("EntitiesPerIpl", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("EntityIpl", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("StaticShadows", "2048"));
	keys.insert(std::pair<std::string, std::string>("Coronas", "20000"));
	keys.insert(std::pair<std::string, std::string>("ScriptSearchLights", "1024"));
	keys.insert(std::pair<std::string, std::string>("FrameLimit", "60"));
	keys.insert(std::pair<std::string, std::string>("MemoryAvailable", "30%"));
	keys.insert(std::pair<std::string, std::string>("StreamingInfo", "unlimited"));
	keys.insert(std::pair<std::string, std::string>("TxdStore", "unlimited"));
	AdjustLimits(keys);
    return 0;
}

 /*
 *  Finds all the limit handlers
 */
void FindHandlers(std::map<std::string, LimitHandler>& handlers)
{
	LimitHandler k;
	auto& adjusters = Adjuster::GetAdjusters();

	// Find all adjusters present on this build and build the map of handlers
	for (auto it = adjusters.begin(); it != adjusters.end(); ++it)
	{
		Adjuster* adjuster = *it;
		if (auto* limits = adjuster->GetLimits())        // Get limits this adjuster can handle
		{
			// For each limit this adjuster can handle...
			for (auto* limit = limits; limit->name; ++limit)
			{
				// ...add it to the handlers map
				k.adjuster = adjuster;
				k.id = limit->id;
				handlers[limit->name] = k;
			}
		}
	}
}

/*
*  Reads an section of limits to be changed
*/
void AdjustLimits(const std::map<std::string, std::string>& section)
{
	// Find the handler for each limit
	FindHandlers(::limits);

	// Parse the section and call the handler for each limit
	for (auto it = section.begin(); it != section.end(); ++it)
	{
		auto pair = limits.find(it->first);   // Find this limit handler
		if (pair != limits.end())
		{
			try
			{
				// Change the limit
				LimitHandler& handler = pair->second;
				handler.adjuster->ChangeLimit(handler.id, it->second);
			}
			catch (const std::exception& ex)
			{
				// ...ops... something wrong happened at ChangeLimit, failed to convert from string to int?
				// Dunno, let's report it to the user
				char buf[512];
				sprintf(buf, "Failed to change limit for '%s' with value '%s'.\nReason: %s\n",
					it->first.c_str(), it->second.c_str(), ex.what());
				MessageBoxA(0, buf, "Limit Adjuster", MB_ICONERROR);
			}
		}
	}

	// Run Process()
	std::map<Adjuster*, int> adjz;   // list of already processed adjusters (int is dummy, too lazy to use std::set)
	for (auto it = limits.begin(); it != limits.end(); ++it)
	{
		auto* adj = it->second.adjuster;
		if (adjz[adj]++ == 0) adj->Process();    // Run only once for each Adjuster*
	}
}



/*
 * -------- Limits Drawing on HUD --------
 */

static injector::hook_back<void(*)()> DrawHUD;
static uint32_t current_limit         = 0;  // Index of the first limit we should draw
static const uint32_t limits_per_page = 18; // Max number of limits in one page
static float currposx;                      // Current drawing position
static float currposy;                      // Current drawing position

struct DrawerTranslator : injector::address_translator
{
    DrawerTranslator()
    {
        map[0x719840] = 0x719840;   // CFont::RenderFontBuffer
        map[0xBAB22C] = 0xBAB22C;   // HudColours
        map[0x58FEA0] = 0x58FEA0;   // CHudColours::GetRGBA
        map[0xC17040] = 0xC17040;   // RsGlobal
        map[0x719380] = 0x719380;   // CFont::SetScale
        map[0x719430] = 0x719430;   // CFont::SetColor
        map[0x719490] = 0x719490;   // CFont::SetFontStyle
        map[0x719510] = 0x719510;   // CFont::SetDropColor
        map[0x719590] = 0x719590;   // CFont::SetEdge
        map[0x7195B0] = 0x7195B0;   // SetProportional
        map[0x7195C0] = 0x7195C0;   // CFont::SetBackground
        map[0x719600] = 0x719600;   // CFont::SetJustify
        map[0x7194F0] = 0x7194F0;   // CFont::SetRightJustifyWrap
        map[0x7194D0] = 0x7194D0;   // CFont::SetWrapx
        map[0x719610] = 0x719610;   // CFont::SetOrientation
        map[0x71A700] = 0x71A700;   // CFont::PrintString
    }
};


// Setups variables to start drawing the limits values
bool BeginDraw()
{
    currposx = 10.0;
    currposy = 105.0;
    return true;
}

// Finishes drawing the limits values
void EndDraw()
{
    static void (*RenderFontBuffer)() = injector::lazy_pointer<0x719840>::get();   // CFont method
    RenderFontBuffer();
}

// Draw a specific text at xy (local screen space) with the specified scale (too)
void DrawText(const char* text, float x, float y, float scalex, float scaley)
{
    // CRGBA structure
    struct CRGBA
    {
        unsigned char red, green, blue, alpha;
        CRGBA() {}
        CRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
            red(r), green(g), blue(b), alpha(a)
        {}
    };



    // Game method pointers
    static void* pInterfaceColour                                           = injector::lazy_pointer<0xBAB22C>::get();   // For GetInterfaceColour
    static void* pGetInterfaceColour                                        = injector::lazy_pointer<0x58FEA0>::get();   // For GetInterfaceColour
    static int*  pRsGlobal                                                  = injector::lazy_pointer<0xC17040>::get();   // To find the resolution
    static void (*SetScale)(float w, float h)                               = injector::lazy_pointer<0x719380>::get();   // CFont method
    static void (*SetColor)(void* color)                                    = injector::lazy_pointer<0x719430>::get();   // CFont method // would work because sizeof(CRGBA) == sizeof(void*)
    static void (*SetFontStyle)(short style)                                = injector::lazy_pointer<0x719490>::get();   // CFont method
    static void (*SetDropColor)(CRGBA color)                                = injector::lazy_pointer<0x719510>::get();   // CFont method
    static void (*SetEdge)(short value)                                     = injector::lazy_pointer<0x719590>::get();   // CFont method
    static void (*SetProportional)(bool on)                                 = injector::lazy_pointer<0x7195B0>::get();   // CFont method
    static void (*SetBackground)(bool background, bool backgroundOnlyText)  = injector::lazy_pointer<0x7195C0>::get();   // CFont method
    static void (*SetJustify)(bool on)                                      = injector::lazy_pointer<0x719600>::get();   // CFont method
    static void (*SetRightJustifyWrap)(float value)                         = injector::lazy_pointer<0x7194F0>::get();   // CFont method
    static void (*SetWrapx)(float value)                                    = injector::lazy_pointer<0x7194D0>::get();   // CFont method
    static void (*SetOrientation)(int alignment)                            = injector::lazy_pointer<0x719610>::get();   // CFont method
    static void (*PrintString)(float x, float y, const char *text)          = injector::lazy_pointer<0x71A700>::get();   // CFont method

    // Gets the interface colour for the text
	CRGBA rgba;
	((CRGBA* (__thiscall *)(void*, CRGBA*, unsigned char))pGetInterfaceColour)(pInterfaceColour, &rgba, 4);


    // Transformer from global screen space to local screen space
    float screenx((float)((signed int)*(pRsGlobal + 1)) / 640.0f);
    float screeny((float)((signed int)*(pRsGlobal + 2)) / 448.0f);

    // Setup the font style
    SetFontStyle(1); 
    SetJustify(0);
    SetBackground(0, 0);
    SetProportional(true);
    SetOrientation(1);
    SetRightJustifyWrap(0);
    SetWrapx(640.0f * screenx);
    SetEdge(1);
    SetDropColor(CRGBA(0, 0, 0, 0xFF));
    SetColor(*(CRGBA**)&rgba);
    SetScale(screenx * scalex, screeny * scaley);

    // Print the text
    PrintString(screenx * x, screeny * y, text);
}

// Draw the specified text and advance the posy to draw the next text
void DrawText(const char* text)
{
    const float x = currposx, y = currposy;             // Pos
    const float sx = 0.60f * 0.65f, sy = 0.89f * 0.65f; // Scale
    currposy += sy * 20;                                // Advance current position
    return DrawText(text, x, y, sx, sy);                // Drawzz
}

// Draw the limit named 'name' with usage counter 'usage'
void DrawLimit(const char* name, const char* usage)
{
    char buffer[1024];
    sprintf(buffer, "%s: %s", name, usage);
    return DrawText(buffer);
}

// Test hotkeys and find out if we should draw the limits on screen
bool TestShouldDraw()
{
    static bool should_draw = false;    // Should draw flag
    static bool prevF5 = false;         // Prev state of the debug text key
    static bool currF5 = false;         // Curr state of the debug text key
    
    // Update key states and find if hot key pressed
    currF5 = (GetKeyState(vkDebugText) & 0x8000) != 0;
    if(currF5 && !prevF5)   // Hotkey pressed?!?
    {
        if(!should_draw)
        {
            // So it should draw since this key press
            current_limit = 0;
            should_draw = true;
        }
        else
        {
            // Go to the next page, or stop drawing the limits
            current_limit += limits_per_page;
            should_draw = current_limit < limits.size();
        }
    }
    prevF5 = currF5;

    return should_draw;
}

// Draw the limits on screen
void DrawLimits()
{
    // Perform CHud::Draw
    DrawHUD.fun? DrawHUD.fun() : void();

    // Draw our list of limits, if it overlaps with the hud (shouldn't) it will be drawn above it
    if(TestShouldDraw() && BeginDraw())
    {
        static DrawerTranslator dwt;
        dwt.enable();

        std::string usage;
        unsigned int i = 0, drawn = 0;  // Limit index and num drawn limits

        for(auto it = limits.begin(); it != limits.end(); ++it)
        {
			auto& pair = *it;

            // Check if are already in the limit we should start drawing from and also 
            // if we are still in the limits per page range OR we didn't draw enought stuff to complete limits per page
            if(i >= current_limit && (i < current_limit + limits_per_page || drawn < limits_per_page))
            {
                if(pair.second.adjuster->GetUsage(pair.second.id, usage))
                {
                    ++drawn;
                    DrawLimit(pair.first.c_str(), usage.c_str());
                }
            }

            ++i;
        }

        EndDraw();
        dwt.disable();
    }
}

// Patches CHud::Draw to draw additional stuff (limits details)
void PatchDrawer()
{
    DrawHUD.fun = injector::MakeCALL(0x53E4FF, DrawLimits).get();
}

void* injector::address_manager::translator(void* p)
{
    // The following must be at the top
    auto& t = address_translator_manager::singleton();

    // Then we need a fallback that will return the original address if no address was found for it
    struct fallbacker : injector::address_translator
    {
        virtual void* fallback(void* p) const
        {
            return p;
        }
    };
    static fallbacker fb;

    return t.translator(p);
}
