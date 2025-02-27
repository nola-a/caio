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

#include <gsl/span>

#include "device.hpp"
#include "types.hpp"


namespace caio {

/**
 * Address Space.
 * This class implements the interface used to access a system's memory mappings.
 * The address space (addr_t) is separated into a number of fixed size banks,
 * each range of addresses inside a bank must be handled by a single device.
 * There are two sets of address space mappings, one for reading and another for writing.
 * @see addr_t
 * @see Device
 */
class ASpace {
public:
    using ReadMode = Device::ReadMode;

    /**
     * Device mappings type.
     * The device mappings type binds a device instance to a memory bank.
     * The first element is a pointer to the device and the second element
     * is the offset within the addr_t address space of the memory bank.
     *
     * The device must handle the entire set of addresses of a bank starting
     * at the specified offset: offset + bank_size <= device.size().
     *
     * When a read or a write is delivered the device receives a relative
     * address from the starting offset.
     *
     * @see devptr_t
     */
    using devmap_t = std::pair<devptr_t, addr_t>;

    /**
     * Address mappings type.
     * The address mappings type defines the device mappings for an entire address space.
     * It must not contain holes (unmapped addresses): The entire addr_t set must be mapped.
     */
    using addrmap_t = gsl::span<devmap_t>;

    /**
     * @see Device::read()
     */
    virtual uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read);

    /**
     * @see Device::write()
     */
    virtual void write(addr_t addr, uint8_t value);

    /**
     * Reset this address space.
     */
    virtual void reset() {
    }

    /**
     * @see Device::peek()
     */
    uint8_t peek(addr_t addr) const {
        return const_cast<ASpace*>(this)->read(addr, ReadMode::Peek);
    }

    /**
     * Read the content of the data bus.
     * This is usually the last value written to the data bus.
     * @return The value present on the data bus.
     */
    uint8_t databus() const {
        return _data;
    }

    /**
     * Set the data bus.
     * Usually the value on the data bus is the one that was last read or
     * written to the address space. This value can be overrided by calling
     * this method.
     * @param data Data to set.
     */
    void databus(uint8_t data) {
        _data = data;
    }

    /**
     * Dump the contents of the read and write mappings as human readable strings.
     * @param os Stream to dump to;
     */
    std::ostream& dump(std::ostream& os) const;

protected:
    ASpace() {
    }

    /**
     * Initialise this address space.
     * @param rmaps Address mappings for read operations (it must have the same size as the write map);
     * @param wmaps Address mappings for write operations (it must have the same size as the read map);
     * @param amask Address space mask (addresses are ANDed with this mask).
     * @note It is expected that all parameters are correct.
     * @see reset()
     */
    ASpace(const addrmap_t& rmaps, const addrmap_t& wmaps, addr_t amask) {
        reset(rmaps, wmaps, amask);
    }

    /**
     * Reset this address space with a new set of mappings.
     * - The size of the address space is 2^(sizeof addr_t);
     * - Both the read an write mappings must be the same size which equals the number of banks;
     * - All banks must have the same size (2^(sizeof addr_t) / banks);
     * - The size of a bank must be a power of 2.
     * @param rmaps Address mappings for read operations;
     * @param wmaps Address mappings for write operations;
     * @param amask Address space mask (addresses are ANDed with this mask).
     * @note It is expected that all parameters are correct.
     */
    void reset(const addrmap_t& rmaps, const addrmap_t& wmaps, addr_t amask);

private:
    std::pair<addr_t, addr_t> decode(addr_t addr) const;

    addr_t    _amask{};
    addr_t    _bsize{};
    addr_t    _bmask{};
    addr_t    _bshift{};
    addrmap_t _rmaps{};
    addrmap_t _wmaps{};
    uint8_t   _data{};
};

}
