/*
 * Paint.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hitrate.hpp"
#include "hack.hpp"

static CatVar cursor_fix_experimental(CV_SWITCH, "experimental_cursor_fix", "1",
                                      "Cursor fix");

void Paint_hook(IEngineVGui *_this, PaintMode_t mode)
{
	static const Paint_t original = (Paint_t)hooks::enginevgui.GetMethod(offsets::PlatformOffset(14, offsets::undefined, offsets::undefined));

    if (!g_IEngine->IsInGame())
        g_Settings.bInvalid = true;

	if (mode & PaintMode_t::PAINT_UIPANELS)
	{
	    hacks::tf2::killstreak::apply_killstreaks();
    	hacks::shared::catbot::update();
        if (hitrate::hitrate_check)
        {
            hitrate::Update();
        }
#if ENABLE_IPC
        static Timer nametimer{};
        if (nametimer.test_and_set(1000 * 10))
        {
            if (ipc::peer)
            {
                ipc::StoreClientData();
            }
        }
        static Timer ipc_timer{};
        if (ipc_timer.test_and_set(1000))
        {
            if (ipc::peer)
            {
                if (ipc::peer->HasCommands())
                {
                    ipc::peer->ProcessCommands();
                }
                ipc::Heartbeat();
                ipc::UpdateTemporaryData();
            }
        }
#endif
        hacks::shared::autojoin::UpdateSearch();
        if (!hack::command_stack().empty())
        {
            PROF_SECTION(PT_command_stack);
            std::lock_guard<std::mutex> guard(hack::command_stack_mutex);
            while (!hack::command_stack().empty())
            {
                //logging::Info("executing %s",
                //              hack::command_stack().top().c_str());
                g_IEngine->ClientCmd_Unrestricted(
                    hack::command_stack().top().c_str());
                hack::command_stack().pop();
            }
        }
#if TEXTMODE_STDIN == 1
        static auto last_stdin = std::chrono::system_clock::from_time_t(0);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now() - last_stdin)
                      .count();
        if (ms > 500)
        {
            UpdateInput();
            last_stdin = std::chrono::system_clock::now();
        }
#endif
#if ENABLE_GUI
        if (cursor_fix_experimental)
        {
            /*			if (gui_visible) {
                            g_ISurface->SetCursorAlwaysVisible(true);
                        } else {
                            g_ISurface->SetCursorAlwaysVisible(false);
                        }*/
        }
#endif
	}

	original(_this, mode);
}