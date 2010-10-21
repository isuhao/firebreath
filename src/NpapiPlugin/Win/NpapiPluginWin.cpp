/**********************************************************\
Original Author: Richard Bateman (taxilian)

Created:    Dec 3, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#include "Win/win_common.h"
#include "NpapiTypes.h"
#include "PluginCore.h"
#include "Win/PluginWindowWin.h"
#include "Win/PluginWindowlessWin.h"
#include "config.h"

#include "Win/NpapiPluginWin.h"

using namespace FB::Npapi;

extern std::string g_dllPath;

NpapiPluginWin::NpapiPluginWin(NpapiBrowserHostPtr host) : NpapiPlugin(host), pluginWin(NULL)
{
    PluginCore::setPlatform("Windows", "NPAPI");
    setFSPath(g_dllPath);
}

NpapiPluginWin::~NpapiPluginWin()
{
    delete pluginWin; pluginWin = NULL;
}

NPError NpapiPluginWin::SetWindow(NPWindow* window)
{
    // If window == NULL then our window is gone. Stop drawing.
    if(window == NULL) {
        // Our window is gone
        if(pluginMain != NULL) {
            // Destroy our FireBreath window
            pluginMain->ClearWindow();
            delete pluginWin; pluginWin = NULL;
        }
        return NPERR_NO_ERROR;
    }

    // Code here diverges depending on if 
    // the plugin is windowed or windowless.
    if(pluginMain->isWindowless()) { 
#if !FB_WINDOWLESS
        return NPERR_GENERIC_ERROR;
#else
        PluginWindowlessWin* win = dynamic_cast<PluginWindowlessWin*>(pluginWin);

        if(win == NULL && pluginWin != NULL) {
            // We've received a window of a different type than the 
            // window we have been using up until now.
            // This is unlikely/impossible, but it's worth checking for.
            pluginMain->ClearWindow();
            delete pluginWin; pluginWin = NULL; 
        }

        if(pluginWin == NULL) {
            // Create new window
            win = _createPluginWindowless((HDC)window->window);
            win->setNpHost(m_npHost);
            win->setWindowPosition(window->x, window->y, window->width, window->height);
            win->setWindowClipping(window->clipRect.top, window->clipRect.left,
                                   window->clipRect.bottom, window->clipRect.right);
            pluginMain->SetWindow(win);
            setReady();
            pluginWin = win;
        } else {
            win->setWindowPosition(window->x, window->y, window->width, window->height);
            win->setWindowClipping(window->clipRect.top, window->clipRect.left,
                                   window->clipRect.bottom, window->clipRect.right);
        }
#endif
    } else { 
        PluginWindowWin* win = dynamic_cast<PluginWindowWin*>(pluginWin);
        // Check to see if we've received a new HWND (new window)
        if(win != NULL && win->getHWND() != (HWND)window->window) {
            pluginMain->ClearWindow();
            delete pluginWin; pluginWin = NULL; 
        }

        if(win == NULL && pluginWin != NULL) {
            // We've received a window of a different type than the 
            // window we have been using up until now.
            // This is unlikely/impossible, but it's worth checking for.
            pluginMain->ClearWindow();
            delete pluginWin; pluginWin = NULL; 
        }
    
        if(pluginWin == NULL) {
            // Reset plugin window
            pluginMain->ClearWindow();
            delete pluginWin; pluginWin = NULL;
            
            // Create new window
            HWND browserHWND;
            m_npHost->GetValue(NPNVnetscapeWindow, (void*)&browserHWND); 
            win = _createPluginWindow((HWND)window->window);
            win->setBrowserHWND(browserHWND);
            pluginMain->SetWindow(win);
            setReady();
            pluginWin = win;
        }    
    }

    return NPERR_NO_ERROR;
}

int16_t NpapiPluginWin::HandleEvent(void* event) {
    PluginWindowlessWin* win = dynamic_cast<PluginWindowlessWin*>(pluginWin);
    if(win != NULL) {
        return win->HandleEvent((NPEvent*)event);
    }
    return false;
}
