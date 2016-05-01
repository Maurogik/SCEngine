/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCEDebug.cpp***********/
/**************************************/

#include "../headers/SCEDefines.hpp"
#include "../headers/SCEDebug.hpp"
#include "../headers/SCETextures.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCEDebugText.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInput.hpp"
#include "../headers/SCERender.hpp"
#include "../headers/SCETime.hpp"
#include "../headers/SCELighting.hpp"
#include "../headers/SCEScene.hpp"

#ifdef SCE_DEBUG_ENGINE

namespace SCE
{
namespace Debug
{
    namespace
    {
        int debugTextures()
        {
            if(SCE::TextureUtils::ToggleDebugTexture())
            {
                return 2;
            }
            return 0;
        }

        int debugTonemap()
        {
            if(SCE::Render::ToggleTonemapOff())
            {
                return 1;
            }
            return 0;
        }

        int debugPauseGame()
        {
            if(SCE::Time::GetTimeSpeed() == 0.0f)
            {
                SCE::Time::SetTimeSpeed(1.0f);
                return 0;
            }
            else
            {
                SCE::Time::SetTimeSpeed(0.0f);
                return 1;
            }
        }

        int debugReloadShaders()
        {
            SCE::ShaderUtils::ReloadShaders();
            return 0;
        }

        int debugReloadMaterialsAndShaders()
        {
            SCE::ShaderUtils::ReloadShaders();
            SCEScene::ReloadAllMaterials();
            return 0;
        }

        typedef int (*debugCallback)();

        bool isEnabled = false;
        std::vector<std::string> menuNames =
        {
            "Toogle texture debugging",            
            "Toogle tonemapping",
            "Pause/Unpause game",
            "Reload Shaders",
            "Reload Materials"
        };
        std::vector<debugCallback> menuCallbacks =
        {
            debugTextures,
            debugTonemap,
            debugPauseGame,
            debugReloadShaders,
            debugReloadMaterialsAndShaders,
        };
        std::vector<int> menuStates = {0, 0, 0, 0, 0};

        glm::vec3 stateColors[] =
        {
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
        };
    }

    bool ToggleDebugMenu()
    {
        isEnabled = !isEnabled;
        return isEnabled;
    }

    void UpdateDebugMenu()
    {
        SCE::Debug::Assert(menuNames.size() == menuCallbacks.size(),
                           "debug menu : menu names count doesn't match menu callbacks");
        SCE::Debug::Assert(menuNames.size() == menuStates.size(),
                           "debug menu : menu names count doesn't match menu states");

        if (SCE::Input::GetKeyAction(GLFW_KEY_GRAVE_ACCENT) == SCE::Input::KeyAction::Press)
        {
            ToggleDebugMenu();
        }

        if(isEnabled)
        {
            float ySpace = 0.03f;
            float yPos = ySpace * float(menuCallbacks.size() + 1);

            SCE::DebugText::RenderString(glm::vec2(0.01f, yPos), "Debug Menu", glm::vec3(1.0, 0.0, 0.0));
            yPos -= ySpace;

            for(uint i = 0; i < menuCallbacks.size(); ++i)
            {
                if (SCE::Input::GetKeyAction(GLFW_KEY_1 + i) == SCE::Input::KeyAction::Press)
                {
                    menuStates[i] = menuCallbacks[i]();
                }

                glm::vec3 color = stateColors[menuStates[i]];
                std::string fullTitle = "PRESS " + std::to_string(i + 1) + " --> " + menuNames[i];
                SCE::DebugText::RenderString(glm::vec2(0.01f, yPos), fullTitle, color);
                yPos -= ySpace;
            }
        }
    }

}
}

#endif
