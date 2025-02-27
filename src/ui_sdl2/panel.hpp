/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#pragma once

#include <tuple>
#include <vector>

#include <SDL.h>

#include "rgb.hpp"
#include "ui_sdl2/widget.hpp"


namespace caio {
namespace ui {
namespace sdl2 {

/**
 * SDL2 Panel.
 * A panel is a container of widgets.
 * @see Widget
 */
class Panel {
public:
    constexpr static const Rgba FRAME_COLOR      = { 0xFF, 0x00, 0x00, 0x80 };
    constexpr static const Rgba BG_COLOR         = { 0x00, 0x00, 0x00, 0x80 };
    constexpr static float WIDTH_RATIO           = 0.618f;
    constexpr static float HEIGHT_RATIO          = WIDTH_RATIO / 4.0f;
    constexpr static const float THICKNESS_RATIO = WIDTH_RATIO / 30.0f;

    enum Just {
        LEFT  = 0,
        RIGHT = 1
    };

    /**
     * Create a Panel.
     * @param renderer Renderer.
     * @exception UIError
     * @see reset()
     */
    Panel(SDL_Renderer* renderer = nullptr);

    virtual ~Panel();

    /**
     * Reset this panel,
     * @param renderer Renderer;
     * @exception UIError
     */
    void reset(SDL_Renderer* renderer = nullptr);

    /**
     * Set the panel visibility.
     * @param is_visible True if visible; false otherwise.
     */
    void visible(bool is_visible);

    /**
     * @return Panel visibility.
     */
    bool visible() const;

    /**
     * Process SDL events.
     * @param event SDL event.
     */
    void event(const SDL_Event& event);

    /**
     * Render the panel.
     * If the panel is visible, render it.
     * @param width  Width of the destination window;
     * @param height Height of the destination window.
     */
    void render(int width, int height);

    /**
     * Add a widget to this panel.
     * @param widget Widget to add;
     * @param just   Justification (Just::LEFT or Just::RIGHT);
     * @return true if the widget was added, false if there is no room for a new widget in the panel.
     */
    void add(const sptr_t<Widget>& widget, Just just = Just::LEFT);

private:
    using just_rect_widget_t = std::tuple<Just, SDL_Rect, sptr_t<Widget>>;

    std::vector<just_rect_widget_t>::const_iterator find_widget(int x, int y);

    bool                            _visible{};         /* Panel visibility                 */
    SDL_Renderer*                   _renderer{nullptr}; /* Renderer                         */
    SDL_Texture*                    _tex{nullptr};      /* Panel texture                    */
    SDL_Rect                        _ext_rect{};        /* Panel rectangle                  */
    sptr_t<Widget>                  _cur_widget{};      /* Widget under the mouse cursor    */
    std::vector<just_rect_widget_t> _widgets{};         /* Panel widgets                    */
};

}
}
}
