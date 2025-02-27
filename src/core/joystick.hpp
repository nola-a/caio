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

#include <cstdint>
#include <string>

#include "name.hpp"


namespace caio {

/**
 * Emulated joystick.
 * This class implements the basics of a joystick and
 * it should be derived by a specific emulated joystick.
 */
class Joystick : public Name {
public:
    constexpr static const char* TYPE          = "JOY";
    constexpr static unsigned JOYID_INVALID    = static_cast<unsigned>(-1);
    constexpr static unsigned JOYID_UNASSIGNED = JOYID_INVALID;
    constexpr static unsigned JOYID_VIRTUAL    = 255;

    enum JoyPosition {
        JOY_NONE  = 0x00,
        JOY_UP    = 0x01,
        JOY_DOWN  = 0x02,
        JOY_RIGHT = 0x04,
        JOY_LEFT  = 0x08,
        JOY_FIRE  = 0x80
    };

    /**
     * Initialise this joystick.
     * @param label Label assigned to this joystick.
     */
    Joystick(const std::string& label = {})
        : Name{TYPE, label} {
    }

    virtual ~Joystick() {
    }

    /**
     * Reset this joystick.
     * This method must be called by the UI to associate
     * this joystick to a real game controller.
     * @param jid Identifier assigned to this joystick.
     */
    virtual void reset(unsigned jid = JOYID_INVALID) {
        _joyid = jid;
        _position = JOY_NONE;
    }

    /**
     * Set the current joystick position.
     * @param pos A bitwise combination of the joystick positions.
     * @see JoyPosition
     */
    virtual void position(uint8_t pos) {
        _position = pos;
    }

    /**
     * @return A bitwise combination of the current joystick positions.
     * @see JoyPosition
     */
    virtual uint8_t position() const {
        return _position;
    }

    /**
     * @return True if there is a real game controller behind this joystick; false otherwise
     */
    bool is_connected() const {
        return (_joyid != JOYID_INVALID);
    }

    /**
     * @return The joystick id.
     */
    unsigned joyid() const {
        return _joyid;
    }

private:
    unsigned _joyid{JOYID_INVALID};
    uint8_t  _position{JOY_NONE};
};

}
