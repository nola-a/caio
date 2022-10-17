/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "zilog_z80.hpp"


namespace caio {

int ZilogZ80::i_IN_A_n(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    self._regs.A = self.in(arg & 0xFF);
    return 0;
}

int ZilogZ80::i_OUT_n_A(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    self.out(arg & 0xFF, self._regs.A);
    return 0;
}

}
