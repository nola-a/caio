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
#include "rgb.hpp"

#include <cstring>
#include <fstream>

#include "utils.hpp"


namespace caio {

Rgba Rgba::transparent{0};

static inline uint8_t adjust(float c)
{
    return (c > 255.0f ? 255 : static_cast<uint8_t>(c));
}

std::string Rgba::to_string() const
{
    std::ostringstream os{};

    os << std::hex << std::setw(2) << std::setfill('0') << +r
       << std::hex << std::setw(2) << std::setfill('0') << +g
       << std::hex << std::setw(2) << std::setfill('0') << +b
       << std::hex << std::setw(2) << std::setfill('0') << +a;

    return os.str();
}

Rgba operator*(const Rgba& color, float value)
{
    return Rgba{
        adjust(color.r * value),
        adjust(color.g * value),
        adjust(color.b * value),
        color.a
    };
}

Rgba operator/(const Rgba& color, float value)
{
    return Rgba{
        adjust(color.r / value),
        adjust(color.g / value),
        adjust(color.b / value),
        color.a
    };
}

Rgba operator+(const Rgba& color1, const Rgba& color2)
{
    return Rgba{
        adjust(color1.r + color2.r),
        adjust(color1.g + color2.g),
        adjust(color1.b + color2.b),
        adjust((color1.a + color2.a) / 2)
    };
}

void RgbaTable::load(const std::string& fname)
{
    std::ifstream is{fname, std::ios::binary | std::ios::in};
    if (!is) {
        throw IOError{"Can't open: " + fname + ": " + Error::to_string()};
    }

    clear();
    std::string line{};
    while (std::getline(is, line)) {
        line = utils::trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        /*
         * Only the rrggbbaa format is supported.
         */
        if (line.size() != 8) {
            throw IOError{fname + ": Invalid line: " + line};
        }

        try {
            auto value = utils::to_number<uint32_t>(line.c_str());
            push_back(Rgba{value});
        } catch (const InvalidNumber&) {
            throw IOError{"Invalid line: " + line};
        }
    }

    is.close();
}

void RgbaTable::save(const std::string& fname)
{
    std::ofstream os{fname, std::ios::binary | std::ios::out | std::ios::trunc};
    if (!os) {
        throw IOError{"Can't create: " + fname + ": " + Error::to_string()};
    }

    for (const auto& rgb : *this) {
        os << rgb.to_string() << std::endl;
        if (!os) {
            throw IOError{"Can't write: " + fname + ": " + Error::to_string()};
        }
    }

    os.close();
}

}
