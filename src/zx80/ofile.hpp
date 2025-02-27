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

#include <string>
#include <vector>

#include <gsl/span>

#include "fs.hpp"
#include "types.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * .o (.80) cassette file image.
 */
class OFile : public std::vector<uint8_t> {
public:
    constexpr static const addr_t SIZE_OFFSET   = 10;
    constexpr static const addr_t LOAD_ADDR     = 0x4000;
    constexpr static const addr_t MAX_SIZE      = 16384;

    OFile() {
    }

    /**
     * Load a cassette image file.
     * @param fname Name of the cassette file.
     * @exception IOError
     */
    OFile(const std::string& fname) {
        load(fname);
    }

    /**
     * Create a cassette file from a data buffer.
     * @param buf Data buffer.
     */
    OFile(std::vector<uint8_t>&& buf)
        : std::vector<uint8_t>{std::move(buf)} {
    }

    virtual ~OFile() {
    }

    /**
     * Load a Cassette Image file.
     * The content of this instance is replaced with the new data.
     * @param fname Name of the file to load.
     * @exception IOError
     */
    void load(const std::string& fname);

    /**
     * Save this cassette image to file.
     * @param fname Name of the file to save.
     * @exception IOError
     */
    void save(const std::string& fname);
};

}
}
}
