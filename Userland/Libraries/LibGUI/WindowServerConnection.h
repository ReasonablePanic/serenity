/*
 * Copyright (c) 2018-2021, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibIPC/ServerConnection.h>
#include <WindowServer/WindowClientEndpoint.h>
#include <WindowServer/WindowServerEndpoint.h>

namespace GUI {

class WindowServerConnection
    : public IPC::ServerConnection<WindowClientEndpoint, WindowServerEndpoint>
    , public WindowClientEndpoint {
    C_OBJECT(WindowServerConnection)
public:
    WindowServerConnection()
        : IPC::ServerConnection<WindowClientEndpoint, WindowServerEndpoint>(*this, "/tmp/portal/window")
    {
        handshake();
    }

    virtual void handshake() override;
    static WindowServerConnection& the();

private:
    virtual void fast_greet(Gfx::IntRect const&, Core::AnonymousBuffer const&) override;
    virtual void paint(i32, Gfx::IntSize const&, Vector<Gfx::IntRect> const&) override;
    virtual void mouse_move(i32, Gfx::IntPoint const&, u32, u32, u32, i32, bool, Vector<String> const&) override;
    virtual void mouse_down(i32, Gfx::IntPoint const&, u32, u32, u32, i32) override;
    virtual void mouse_double_click(i32, Gfx::IntPoint const&, u32, u32, u32, i32) override;
    virtual void mouse_up(i32, Gfx::IntPoint const&, u32, u32, u32, i32) override;
    virtual void mouse_wheel(i32, Gfx::IntPoint const&, u32, u32, u32, i32) override;
    virtual void window_entered(i32) override;
    virtual void window_left(i32) override;
    virtual void key_down(i32, u32, u32, u32, u32) override;
    virtual void key_up(i32, u32, u32, u32, u32) override;
    virtual void window_activated(i32) override;
    virtual void window_deactivated(i32) override;
    virtual void window_input_entered(i32) override;
    virtual void window_input_left(i32) override;
    virtual void window_close_request(i32) override;
    virtual void window_resized(i32, Gfx::IntRect const&) override;
    virtual void menu_item_activated(i32, u32) override;
    virtual void menu_item_entered(i32, u32) override;
    virtual void menu_item_left(i32, u32) override;
    virtual void menu_visibility_did_change(i32, bool) override;
    virtual void screen_rect_changed(Gfx::IntRect const&) override;
    virtual void set_wallpaper_finished(bool) override;
    virtual void drag_dropped(i32, Gfx::IntPoint const&, String const&, HashMap<String, ByteBuffer> const&) override;
    virtual void drag_accepted() override;
    virtual void drag_cancelled() override;
    virtual void update_system_theme(Core::AnonymousBuffer const&) override;
    virtual void window_state_changed(i32, bool, bool) override;
    virtual void display_link_notification() override;
    virtual void ping() override;

    bool m_display_link_notification_pending { false };
};

}
