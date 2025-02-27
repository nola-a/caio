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
#include "zilog_z80.hpp"


namespace caio {
namespace zilog {

int Z80::i_LD_rr_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD BC, nn
     * LD DE, nn
     * LD HL, nn
     * LD SP, nn
     */
    auto [rget, rset] = self.reg16_from_opcode(op);
    rset(arg);
    self._regs.memptr = arg + 1;
    return 0;
}

int Z80::i_LD_r_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD A, n
     * LD B, n
     * LD C, n
     * LD D, n
     * LD E, n
     * LD H, n
     * LD L, n
     */
    uint8_t& reg = self.reg8_from_opcode(op);
    reg = static_cast<uint8_t>(arg & 0xFF);
    return 0;
}

int Z80::i_LD_r_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, {ABCDEHL}
     */
    uint8_t& dst_reg = self.reg8_from_opcode(op);
    uint8_t& src_reg = self.reg8_src_from_opcode(op);
    dst_reg = src_reg;
    return 0;
}

int Z80::i_LD_r_mHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, (HL)
     */
    uint8_t& dst_reg = self.reg8_from_opcode(op);
    uint8_t data = self.read(self._regs.HL());
    dst_reg = data;
    return 0;
}

int Z80::i_LD_mHL_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (HL), {ABCDEHL}
     */
    uint8_t& src_reg = self.reg8_src_from_opcode(op);
    self.write(self._regs.HL(), src_reg);
    return 0;
}

int Z80::i_LD_A_mdd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD A, (BC) - $0A
     * LD A, (DE) - $1A
     * LD A, (nn) - $3A
     */
    addr_t addr = (op == 0x0A ? self._regs.BC() : (op == 0x1A ? self._regs.DE() : arg));
    uint8_t data = self.read(addr);
    self._regs.A = data;
    self._regs.memptr = addr + 1;
    return 0;
}

int Z80::i_LD_mdd_A(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (BC), A - $02
     * LD (DE), A - $12
     * LD (nn), A - $32
     */
    addr_t addr = (op == 0x02 ? self._regs.BC() : (op == 0x12 ? self._regs.DE() : arg));
    self.write(addr, self._regs.A);
    self._regs.memptr = (static_cast<uint16_t>(self._regs.A) << 8) | ((addr + 1) & 0xFF);
    return 0;
}

int Z80::i_LD_mnn_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (nn), HL
     */
    self.write(arg, self._regs.L);
    self.write(arg + 1, self._regs.H);
    self._regs.memptr = arg + 1;
    return 0;
}

int Z80::i_LD_HL_mnn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD HL, (nn)
     */
    self._regs.L = self.read(arg);
    self._regs.H = self.read(arg + 1);
    self._regs.memptr = arg + 1;
    return 0;
}

int Z80::i_LD_mHL_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (HL), n
     */
    self.write(self._regs.HL(), static_cast<uint8_t>(arg & 0xFF));
    return 0;
}

int Z80::i_LD_SP_HL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD SP, HL
     */
    self._regs.SP = self._regs.HL();
    return 0;
}

int Z80::i_POP_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * POP {BC,DE,HL,AF}
     */
    auto [rget, rset] = self.reg16_from_opcode(op, true /* no SP */);
    rset(self.pop_addr());
    return 0;
}

int Z80::i_PUSH_rr(Z80& self, uint8_t op, addr_t arg)
{
    auto [rget, rset] = self.reg16_from_opcode(op, true /* no SP */);
    self.push_addr(rget());
    return 0;
}

}
}
