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

#include "fs.hpp"


using namespace std::literals::string_literals;

namespace caio {
namespace zilog {

/*
 * Most of the information in the following table come from:
 * - Z80 CPU User Manual UM008011-0816.
 * - https://clrhome.org/table/
 */
const std::array<Z80::Instruction, 256> Z80::instr_set{{
    { "NOP",            Z80::i_NOP,         ArgType::None,  4,  1   },  /* 00 */
    { "LD BC, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* 01 */
    { "LD (BC), A",     Z80::i_LD_mdd_A,    ArgType::None,  7,  1   },  /* 02 */
    { "INC BC",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* 03 */
    { "INC B",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 04 */
    { "DEC B",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 05 */
    { "LD B, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  1   },  /* 06 */
    { "RLCA",           Z80::i_RLCA,        ArgType::None,  4,  2   },  /* 07 */
    { "EX AF, AF'",     Z80::i_EX_AF_sAF,   ArgType::None,  4,  1   },  /* 08 */
    { "ADD HL, BC",     Z80::i_ADD_HL_rr,   ArgType::None,  11, 1   },  /* 09 */
    { "LD A, (BC)",     Z80::i_LD_A_mdd,    ArgType::None,  7,  1   },  /* 0A */
    { "DEC BC",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* 0B */
    { "INC C",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 0C */
    { "DEC C",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 0D */
    { "LD C, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* 0E */
    { "RRCA",           Z80::i_RRCA,        ArgType::None,  4,  1   },  /* 0F */

    { "DJNZ $+",        Z80::i_DJNZ,        ArgType::A8,    13, 2   },  /* 10 */
    { "LD DE, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* 11 */
    { "LD (DE), A",     Z80::i_LD_mdd_A,    ArgType::None,  7,  1   },  /* 12 */
    { "INC DE",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* 13 */
    { "INC D",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 14 */
    { "DEC D",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 15 */
    { "LD D, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* 16 */
    { "RLA" ,           Z80::i_RLA,         ArgType::None,  4,  1   },  /* 17 */
    { "JR $+" ,         Z80::i_JR,          ArgType::A8,    12, 1   },  /* 18 */
    { "ADD HL, DE",     Z80::i_ADD_HL_rr,   ArgType::None,  11, 1   },  /* 19 */
    { "LD A, (DE)",     Z80::i_LD_A_mdd,    ArgType::None,  7,  1   },  /* 1A */
    { "DEC DE",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* 1B */
    { "INC E",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 1C */
    { "DEC E",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 1D */
    { "LD E, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* 1E */
    { "RRA",            Z80::i_RRA,         ArgType::None,  4,  1   },  /* 1F */

    { "JR NZ, $+",      Z80::i_JR_NZ,       ArgType::A8,    12, 2   },  /* 20 */
    { "LD HL, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* 21 */
    { "LD ($^), HL",    Z80::i_LD_mnn_HL,   ArgType::A16,   16, 3   },  /* 22 */
    { "INC HL",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* 23 */
    { "INC H",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 24 */
    { "DEC H",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 25 */
    { "LD H, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* 26 */
    { "DAA",            Z80::i_DAA,         ArgType::None,  4,  1   },  /* 27 */
    { "JR Z, $+",       Z80::i_JR_Z,        ArgType::A8,    12, 2   },  /* 28 */
    { "ADD HL, HL",     Z80::i_ADD_HL_rr,   ArgType::None,  11, 1   },  /* 29 */
    { "LD HL, ($^)",    Z80::i_LD_HL_mnn,   ArgType::A16,   16, 3   },  /* 2A */
    { "DEC HL",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* 2B */
    { "INC L",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 2C */
    { "DEC L",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 2D */
    { "LD L, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* 2E */
    { "CPL",            Z80::i_CPL,         ArgType::None,  4,  1   },  /* 2F */

    { "JR NC, $+",      Z80::i_JR_NC,       ArgType::A8,    12, 2   },  /* 30 */
    { "LD SP, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* 31 */
    { "LD ($^), A",     Z80::i_LD_mdd_A,    ArgType::A16,   13, 3   },  /* 32 */
    { "INC SP",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* 33 */
    { "INC (HL)",       Z80::i_INC_mHL,     ArgType::None,  11, 1   },  /* 34 */
    { "DEC (HL)",       Z80::i_DEC_mHL,     ArgType::None,  11, 1   },  /* 35 */
    { "LD (HL), $*",    Z80::i_LD_mHL_n,    ArgType::A8,    10, 2   },  /* 36 */
    { "SCF",            Z80::i_SCF,         ArgType::None,  4,  1   },  /* 37 */
    { "JR C, $+",       Z80::i_JR_C,        ArgType::A8,    12, 2   },  /* 38 */
    { "ADD HL, SP",     Z80::i_ADD_HL_rr,   ArgType::None,  11, 1   },  /* 39 */
    { "LD A, ($^)",     Z80::i_LD_A_mdd,    ArgType::A16,   13, 3   },  /* 3A */
    { "DEC SP",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* 3B */
    { "INC A",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* 3C */
    { "DEC A",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* 3D */
    { "LD A, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* 3E */
    { "CCF",            Z80::i_CCF,         ArgType::None,  4,  1   },  /* 3F */

    { "LD B, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 40 */
    { "LD B, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 41 */
    { "LD B, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 42 */
    { "LD B, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 43 */
    { "LD B, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 44 */
    { "LD B, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 45 */
    { "LD B, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 46 */
    { "LD B, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 47 */
    { "LD C, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 48 */
    { "LD C, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 49 */
    { "LD C, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 4A */
    { "LD C, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 4B */
    { "LD C, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 4C */
    { "LD C, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 4D */
    { "LD C, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 4E */
    { "LD C, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 4F */

    { "LD D, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 50 */
    { "LD D, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 51 */
    { "LD D, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 52 */
    { "LD D, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 53 */
    { "LD D, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 54 */
    { "LD D, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 55 */
    { "LD D, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 56 */
    { "LD D, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 57 */
    { "LD E, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 58 */
    { "LD E, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 59 */
    { "LD E, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 5A */
    { "LD E, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 5B */
    { "LD E, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 5C */
    { "LD E, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 5D */
    { "LD E, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 5E */
    { "LD E, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 5F */

    { "LD H, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 60 */
    { "LD H, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 61 */
    { "LD H, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 62 */
    { "LD H, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 63 */
    { "LD H, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 64 */
    { "LD H, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 65 */
    { "LD H, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 66 */
    { "LD H, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 67 */
    { "LD L, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 68 */
    { "LD L, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 69 */
    { "LD L, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 6A */
    { "LD L, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 6B */
    { "LD L, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 6C */
    { "LD L, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 6D */
    { "LD L, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 6E */
    { "LD L, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 6F */

    { "LD (HL), B",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 70 */
    { "LD (HL), C",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 71 */
    { "LD (HL), D",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 72 */
    { "LD (HL), E",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 73 */
    { "LD (HL), H",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 74 */
    { "LD (HL), L",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 75 */
    { "HALT",           Z80::i_HALT,        ArgType::None,  4,  1   },  /* 76 */
    { "LD (HL), A",     Z80::i_LD_mHL_r,    ArgType::None,  7,  1   },  /* 77 */
    { "LD A, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 78 */
    { "LD A, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 79 */
    { "LD A, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 7A */
    { "LD A, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 7B */
    { "LD A, H",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 7C */
    { "LD A, L",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 7D */
    { "LD A, (HL)",     Z80::i_LD_r_mHL,    ArgType::None,  7,  1   },  /* 7E */
    { "LD A, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* 7F */

    { "ADD A, B",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 80 */
    { "ADD A, C",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 81 */
    { "ADD A, D",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 82 */
    { "ADD A, E",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 83 */
    { "ADD A, H",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 84 */
    { "ADD A, L",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 85 */
    { "ADD A, (HL)",    Z80::i_ADD_A_mHL,   ArgType::None,  7,  1   },  /* 86 */
    { "ADD A, A",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* 87 */
    { "ADC A, B",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 88 */
    { "ADC A, C",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 89 */
    { "ADC A, D",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 8A */
    { "ADC A, E",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 8B */
    { "ADC A, H",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 8C */
    { "ADC A, L",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 8D */
    { "ADC A, (HL)",    Z80::i_ADC_A_mHL,   ArgType::None,  7,  1   },  /* 8E */
    { "ADC A, A",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* 8F */

    { "SUB B",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 90 */
    { "SUB C",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 91 */
    { "SUB D",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 92 */
    { "SUB E",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 93 */
    { "SUB H",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 94 */
    { "SUB L",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 95 */
    { "SUB (HL)",       Z80::i_SUB_A_mHL,   ArgType::None,  7,  1   },  /* 96 */
    { "SUB A",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* 97 */
    { "SBC B",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 98 */
    { "SBC C",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 99 */
    { "SBC D",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 9A */
    { "SBC E",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 9B */
    { "SBC H",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 9C */
    { "SBC L",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 9D */
    { "SBC (HL)",       Z80::i_SBC_A_mHL,   ArgType::None,  7,  1   },  /* 9E */
    { "SBC A",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* 9F */

    { "AND B",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A0 */
    { "AND C",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A1 */
    { "AND D",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A2 */
    { "AND E",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A3 */
    { "AND H",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A4 */
    { "AND L",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A5 */
    { "AND (HL)",       Z80::i_AND_A_mHL,   ArgType::None,  7,  1   },  /* A6 */
    { "AND A",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* A7 */
    { "XOR B",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* A8 */
    { "XOR C",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* A9 */
    { "XOR D",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* AA */
    { "XOR E",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* AB */
    { "XOR H",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* AC */
    { "XOR L",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* AD */
    { "XOR (HL)",       Z80::i_XOR_A_mHL,   ArgType::None,  7,  1   },  /* AE */
    { "XOR A",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* AF */

    { "OR B",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B0 */
    { "OR C",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B1 */
    { "OR D",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B2 */
    { "OR E",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B3 */
    { "OR H",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B4 */
    { "OR L",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B5 */
    { "OR (HL)",        Z80::i_OR_A_mHL,    ArgType::None,  7,  1   },  /* B6 */
    { "OR A",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* B7 */
    { "CP B",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* B8 */
    { "CP C",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* B9 */
    { "CP D",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* BA */
    { "CP E",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* BB */
    { "CP H",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* BC */
    { "CP L",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* BD */
    { "CP (HL)",        Z80::i_CP_A_mHL,    ArgType::None,  7,  1   },  /* BE */
    { "CP A",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* BF */

    { "RET NZ",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* C0 */
    { "POP BC",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* C1 */
    { "JP NZ, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* C2 */
    { "JP $^",          Z80::i_JP_nn,       ArgType::A16,   10, 3   },  /* C3 */
    { "CALL NZ, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* C4 */
    { "PUSH BC",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* C5 */
    { "ADD A, $*",      Z80::i_ADD_A_n,     ArgType::A8,    7,  2   },  /* C6 */
    { "RST $00",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* C7 */
    { "RET Z",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* C8 */
    { "RET",            Z80::i_RET,         ArgType::None,  10, 1   },  /* C9 */
    { "JP Z, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* CA */
    { "",               {},                 ArgType::Bit,   0,  0   },  /* CB */
    { "CALL Z, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* CC */
    { "CALL $^",        Z80::i_CALL_nn,     ArgType::A16,   17, 3   },  /* CD */
    { "ADC A, $*",      Z80::i_ADC_A_n,     ArgType::A8,    7,  2   },  /* CE */
    { "RST $08",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* CF */

    { "RET NC",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* D0 */
    { "POP DE",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* D1 */
    { "JP NC, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* D2 */
    { "OUT ($*), A",    Z80::i_OUT_n_A,     ArgType::A8,    11, 2   },  /* D3 */
    { "CALL NC, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* D4 */
    { "PUSH DE",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* D5 */
    { "SUB $*",         Z80::i_SUB_A_n,     ArgType::A8,    7,  2   },  /* D6 */
    { "RST $10",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* D7 */
    { "RET C",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* D8 */
    { "EXX",            Z80::i_EXX,         ArgType::None,  4,  1   },  /* D9 */
    { "JP C, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DA */
    { "IN A, ($*)",     Z80::i_IN_A_n,      ArgType::A8,    11, 2   },  /* DB */
    { "CALL C, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DC */
    { "",               {},                 ArgType::IX,    0,  0   },  /* DD */
    { "SBC A, $*",      Z80::i_SBC_A_n,     ArgType::A8,    7,  2   },  /* DE */
    { "RST $18",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DF */

    { "RET PO",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* E0 */
    { "POP HL",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* E1 */
    { "JP PO, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* E2 */
    { "EX (SP), HL",    Z80::i_EX_mSP_HL,   ArgType::None,  19, 1   },  /* E3 */
    { "CALL PO, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* E4 */
    { "PUSH HL",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* E5 */
    { "AND $*",         Z80::i_AND_A_n,     ArgType::A8,    7,  2   },  /* E6 */
    { "RST $20",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* E7 */
    { "RET PE",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* E8 */
    { "JP (HL)",        Z80::i_JP_HL,       ArgType::None,  4,  1   },  /* E9 */
    { "JP PE, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* EA */
    { "EX DE, HL",      Z80::i_EX_DE_HL,    ArgType::None,  4,  1   },  /* EB */
    { "CALL PE, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* EC */
    { "",               {},                 ArgType::MI,    0,  0   },  /* ED */
    { "XOR $*",         Z80::i_XOR_A_n,     ArgType::A8,    7,  2   },  /* EE */
    { "RST $28",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* EF */

    { "RET P",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* F0 */
    { "POP AF",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* F1 */
    { "JP P, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* F2 */
    { "DI",             Z80::i_DI,          ArgType::None,  4,  1   },  /* F3 */
    { "CALL P, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* F4 */
    { "PUSH AF",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* F5 */
    { "OR $*",          Z80::i_OR_A_n,      ArgType::A8,    7,  2   },  /* F6 */
    { "RST $30",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* F7 */
    { "RET M",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* F8 */
    { "LD SP, HL",      Z80::i_LD_SP_HL,    ArgType::None,  6,  1   },  /* F9 */
    { "JP M, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FA */
    { "EI",             Z80::i_EI,          ArgType::None,  4,  1   },  /* FB */
    { "CALL M, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FC */
    { "",               {},                 ArgType::IY,    0,  0   },  /* FD */
    { "CP $*",          Z80::i_CP_A_n,      ArgType::A8,    7,  2   },  /* FE */
    { "RST $38",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FF */
}};

void Z80::Registers::AF(uint16_t val)
{
    A = val >> 8;
    F = val & 255;
}

void Z80::Registers::BC(uint16_t val)
{
    B = val >> 8;
    C = val & 255;
}

void Z80::Registers::DE(uint16_t val)
{
    D = val >> 8;
    E = val & 255;
}

void Z80::Registers::HL(uint16_t val)
{
    H = val >> 8;
    L = val & 255;
}

void Z80::Registers::aAF(uint16_t val)
{
    aA = val >> 8;
    aF = val & 255;
}

void Z80::Registers::aBC(uint16_t val)
{
    aB = val >> 8;
    aC = val & 255;
}

void Z80::Registers::aDE(uint16_t val)
{
    aD = val >> 8;
    aE = val & 255;
}

void Z80::Registers::aHL(uint16_t val)
{
    aH = val >> 8;
    aL = val & 255;
}

std::string Z80::Registers::to_string(Z80::Flags fl)
{
    std::ostringstream ss{};

    ss << ((fl & Flags::S) ? "S" : "-")
       << ((fl & Flags::Z) ? "Z" : "-")
       << ((fl & Flags::Y) ? "Y" : "-")     /* Undocumented */
       << ((fl & Flags::H) ? "H" : "-")
       << ((fl & Flags::X) ? "X" : "-")     /* Undocumented */
       << ((fl & Flags::V) ? "V" : "-")
       << ((fl & Flags::N) ? "N" : "-")
       << ((fl & Flags::C) ? "C" : "-");

    return ss.str();
}

std::string Z80::Registers::to_string() const
{
    std::ostringstream ss{};

    ss << "   A="    << utils::to_string(A)
       << "  B="     << utils::to_string(B)
       << "  C="     << utils::to_string(C)
       << "  D="     << utils::to_string(D)
       << "  E="     << utils::to_string(E)
       << "  H="     << utils::to_string(H)
       << "  L="     << utils::to_string(L)
       << "  F="     << utils::to_string(F)  << " " << to_string(static_cast<Flags>(F)) << std::endl
       << "  A'="    << utils::to_string(aA)
       << " B'="     << utils::to_string(aB)
       << " C'="     << utils::to_string(aC)
       << " D'="     << utils::to_string(aD)
       << " E'="     << utils::to_string(aE)
       << " H'="     << utils::to_string(aH)
       << " L'="     << utils::to_string(aL)
       << " F'="     << utils::to_string(aF) << " " << to_string(static_cast<Flags>(aF)) << std::endl
       << "  I="     << utils::to_string(I)
       << " R="      << utils::to_string(R)
       << " IX="     << utils::to_string(IX)
       << " IY="     << utils::to_string(IY)
       << " SP="     << utils::to_string(SP)
       << " PC="     << utils::to_string(PC)
       << " memptr=" << utils::to_string(memptr);

    return ss.str();
}

void Z80::init(const sptr_t<ASpace>& mmap)
{
    if (mmap) {
        _mmap = mmap;
    }

    Z80::reset();
}

void Z80::init_monitor(int ifd, int ofd, const monitor::load_cb& load, const monitor::save_cb& save)
{
    using namespace gsl;
    Expects(ifd >= 0 && ofd >= 0);

    auto pc = [this]() -> addr_t& {
        return this->_regs.PC;
    };

    auto mmap = [this]() {
        return this->_mmap;
    };

    auto regvalue = [this](const std::string& rname) -> uint16_t {
        static std::map<std::string, std::function<int(const Z80&)>> regvals{
            { "ra",    [](const Z80& cpu) { return cpu._regs.A;        }},
            { "rf",    [](const Z80& cpu) { return cpu._regs.F;        }},
            { "raf",   [](const Z80& cpu) { return cpu._regs.AF();     }},
            { "rb",    [](const Z80& cpu) { return cpu._regs.B;        }},
            { "rc",    [](const Z80& cpu) { return cpu._regs.C;        }},
            { "rbc",   [](const Z80& cpu) { return cpu._regs.BC();     }},
            { "rd",    [](const Z80& cpu) { return cpu._regs.D;        }},
            { "re",    [](const Z80& cpu) { return cpu._regs.E;        }},
            { "rde",   [](const Z80& cpu) { return cpu._regs.DE();     }},
            { "rh",    [](const Z80& cpu) { return cpu._regs.H;        }},
            { "rl",    [](const Z80& cpu) { return cpu._regs.L;        }},
            { "rhl",   [](const Z80& cpu) { return cpu._regs.HL();     }},
            { "ra'",   [](const Z80& cpu) { return cpu._regs.aA;       }},
            { "rf'",   [](const Z80& cpu) { return cpu._regs.aF;       }},
            { "raf'",  [](const Z80& cpu) { return cpu._regs.aAF();    }},
            { "rb'",   [](const Z80& cpu) { return cpu._regs.aB;       }},
            { "rc'",   [](const Z80& cpu) { return cpu._regs.aC;       }},
            { "rbc'",  [](const Z80& cpu) { return cpu._regs.aBC();    }},
            { "rd'",   [](const Z80& cpu) { return cpu._regs.aD;       }},
            { "re'",   [](const Z80& cpu) { return cpu._regs.aE;       }},
            { "rde'",  [](const Z80& cpu) { return cpu._regs.aDE();    }},
            { "rh'",   [](const Z80& cpu) { return cpu._regs.aH;       }},
            { "rl'",   [](const Z80& cpu) { return cpu._regs.aL;       }},
            { "rhl'",  [](const Z80& cpu) { return cpu._regs.aHL();    }},
            { "ri",    [](const Z80& cpu) { return cpu._regs.I;        }},
            { "rr",    [](const Z80& cpu) { return cpu._regs.R;        }},
            { "rx",    [](const Z80& cpu) { return cpu._regs.IX;       }},
            { "ry",    [](const Z80& cpu) { return cpu._regs.IY;       }},
            { "rsp",   [](const Z80& cpu) { return cpu._regs.SP;       }},
            { "rpc",   [](const Z80& cpu) { return cpu._regs.PC;       }},
            { "rf.s",  [](const Z80& cpu) { return cpu.test_S();       }},
            { "rf.z",  [](const Z80& cpu) { return cpu.test_Z();       }},
            { "rf.h",  [](const Z80& cpu) { return cpu.test_H();       }},
            { "rf.v",  [](const Z80& cpu) { return cpu.test_V();       }},
            { "rf.n",  [](const Z80& cpu) { return cpu.test_N();       }},
            { "rf.c",  [](const Z80& cpu) { return cpu.test_C();       }},
            { "rf'.s", [](const Z80& cpu) { return cpu.test_aS();      }},
            { "rf'.z", [](const Z80& cpu) { return cpu.test_aZ();      }},
            { "rf'.h", [](const Z80& cpu) { return cpu.test_aH();      }},
            { "rf'.v", [](const Z80& cpu) { return cpu.test_aV();      }},
            { "rf'.n", [](const Z80& cpu) { return cpu.test_aN();      }},
            { "rf'.c", [](const Z80& cpu) { return cpu.test_aC();      }}
        };
        auto it = regvals.find(rname);
        if (it != regvals.end()) {
            return it->second(*this);
        }
        throw InvalidArgument{};
    };

    auto bpdoc = [](const std::string& cmd) -> std::string {
        return {
            cmd + " help | h | ?\n" +
            cmd + " <addr> [<cond>]\n\n"
            "<cond> = <val> <op> <val>\n"
            "<val>  = [*]{[#][$]<u16> |\n"
            "         ra | rf | raf | rb | rc | rbc | rd | re | rde | rh | rl | rhl |\n"
            "         ra' | rf' | raf' | rb' | rc' | rbc' | rd' | re' | rde' | rh' | rl' | rhl' |\n"
            "         ri | rr | rx | rsp | rpc |\n"
            "         rf.s | rf.z | rf.z | rf.h | rf.v | rf.n | rf.c |\n"
            "         rf'.s | rf'.z | rf'.z | rf'.h | rf'.v | rf'.n | rf'.c}\n"
            "<op>   = '<' | '>' | '<=' | '>=' | '==' | '!=' | '&' | '|'\n\n"
            "examples:\n"
            "  b $8009 *$fd20 >= #$f0\n"
            "  b $8010 ra >= 80\n"
            "  b $4100 rf.c == 1\n"
        };
    };

    MonitoredCPU monitor_funcs = monitor::monitored_cpu_defaults(this);
    monitor_funcs.pc = pc;
    monitor_funcs.mmap = mmap;
    monitor_funcs.regvalue = regvalue;
    monitor_funcs.bpdoc = bpdoc;

    if (load) {
        monitor_funcs.load = load;
    }

    if (save) {
        monitor_funcs.save = save;
    }

    _monitor = std::make_unique<Monitor>(ifd, ofd, std::move(monitor_funcs));
    _monitor->add_breakpoint(vRESET);
}

void Z80::loglevel(const std::string& lvs)
{
    if (!empty(lvs)) {
        _log.loglevel(lvs);
    }
}

Logger::Level Z80::loglevel() const
{
    return _log.loglevel();
}

void Z80::halt_pin(const OutputPinCb& halt_cb)
{
    _halt_cb = halt_cb;
}

void Z80::iorq_pin(const OutputPinCb& iorq_cb)
{
    _iorq_cb = iorq_cb;
}

void Z80::iorq_pin(bool active)
{
    if (active != _iorq_pin) {
        _iorq_pin = active;
        if (_iorq_cb) {
            _iorq_cb(active);
        }
    }
}

bool Z80::iorq_pin() const
{
    return _iorq_pin;
}

void Z80::m1_pin(const OutputPinCb& m1_cb)
{
    _m1_cb = m1_cb;
}

void Z80::m1_pin(bool active)
{
    if (active != _m1_pin) {
        _m1_pin = active;
        if (_m1_cb) {
            _m1_cb(active);
        }
    }
}

bool Z80::m1_pin() const
{
    return _m1_pin;
}

void Z80::rfsh_pin(const OutputPinCb& rfsh_cb)
{
    _rfsh_cb = rfsh_cb;
}

void Z80::rfsh_pin(bool active)
{
    if (active != _rfsh_pin) {
        _rfsh_pin = active;
        if (_rfsh_cb) {
            _rfsh_cb(active);
        }
    }
}

bool Z80::rfsh_pin() const
{
    return _rfsh_pin;
}

inline bool Z80::wait_pin() const
{
    return _wait_pin;
}

void Z80::halt_pin(bool active)
{
    if (active != _halted) {
        _halted = active;
        if (_halt_cb) {
            _halt_cb(_halted);
        }
    }
}

bool Z80::halt_pin() const
{
    return _halted;
}

void Z80::int_pin(bool active)
{
    _int_pin = active;
}

void Z80::nmi_pin(bool active)
{
    _nmi_pin = active;
}

void Z80::reset_pin(bool active)
{
    if (_reset_pin != active) {
        if (active) {
            reset();
        }
    }

    _reset_pin = active;
}

void Z80::wait_pin(bool active)
{
    _wait_pin = active;
}

void Z80::reset()
{
    _imode = IMODE_0;
    _IFF1  = false;
    _IFF2  = false;

    _regs = {
        .A  = 0xFF,
        .F  = 0xFF,
        .I  = 0,
        .R  = 0,
        .SP = 0xFFFF,
        .PC = vRESET,
    };

    halt_pin(false);
    m1_pin(false);
    rfsh_pin(false);

    _tx = Cycle::T1;
}

size_t Z80::m1_cycle()
{
    size_t cycles{};
    addr_t rfsh_addr{};
    uint8_t opcode{};
    bool forced{};

    switch (_tx) {
    case Cycle::T1:
        /*
         * Initiate the M1 cycle.
         */
        m1_pin(true);
        rfsh_pin(false);
        _tx = Cycle::T2;
        return 1;

    case Cycle::T2:
        /*
         * Add wait-states (if requested by external devices).
         */
        if (wait_pin()) {
            _log.debug("Wait state\n");
        } else {
            _tx = Cycle::T3;
        }
        return 1;

    case Cycle::T3:
        /*
         * Fetch the opcode.
         */
        if (_halted) {
            opcode = I_NOP; /* NOP operation forced on halt state */
            forced = true;
        } else {
            opcode = read(_regs.PC);
            forced = false;
        }

        /*
         * End the M1 cycle.
         */
        m1_pin(false);

        /*
         * The R register is incremented after the opcode is fetched.
         * (Bit 8 remains untouched).
         */
        _regs.R = (_regs.R & 0x80) | (((_regs.R & 0x7F) + 1) & 0x7F);

        /* The Undocumented Z80 Documented, p23 */
        if (opcode == I_BIT || opcode == I_IX || opcode == I_MI || opcode == I_IY) {
            _regs.R = (_regs.R & 0x80) | (((_regs.R & 0x7F) + 1) & 0x7F);
        }

        /*
         * Execute the fetched instruction.
         */
        cycles = execute(opcode, forced);
        if (cycles == 0) {
            return 0;
        }

        /*
         * Start the refresh cycle.
         */
        rfsh_pin(true);
        rfsh_addr = (static_cast<addr_t>(_regs.I) << 8) | _regs.R;
        read(rfsh_addr);    /* Force rfsh_addr on the address bus */

        /*
         * The cycles variable include the timing for T1, T2 and Tn.
         * Times for T1 and T2 have been consumed by the clock.
         * Tn takes place during the next call to this method and its time will be consumed separately.
         * But,
         * "When an EI instruction is executed, any pending interrupt request is not accepted
         * until after the instruction following EI is executed. This single instruction delay
         * is necessary when the next instruction is a return instruction." (z80cpu_um.pdf p18).
         */
        if (opcode == I_EI) {
            _tx = Cycle::T1;
            return (cycles - 2);
        }

        _tx = Cycle::Tn;
        return (cycles - 3);

    case Cycle::Tn:
        /*
         * Inpterrupt pins sampled on last instruction cycle.
         */
        if (_nmi_pin) {
            _is_nmi = true;
        } else if (_IFF1 && _int_pin) {
            _is_int = true;
        }

        _tx = Cycle::T1;
        return 1;
    }
}

size_t Z80::m1_cycle_interrupt()
{
    switch (_tx) {
    case Cycle::T1:
        /*
         * Initiate the special M1 cycle.
         */
        m1_pin(true);
        rfsh_pin(false);
        halt_pin(false);

        if (_is_int) {
            iorq_pin(true);
        }

        _tx = Cycle::T2;
        return 1;

    case Cycle::T2:
        /*
         * Add wait-states for /INT (if requested by external devices).
         */
        if (_is_int && wait_pin()) {
            _log.debug("INT wait state\n");
        } else {
            _tx = Cycle::T3;
        }
        return 1;

    case Cycle::T3:
    default:
        /*
         * End the special M1 cycle.
         */
        m1_pin(false);

        /*
         * The R register is incremented at T3 (bit 8 remains untouched).
         */
        _regs.R = (_regs.R & 0x80) | (((_regs.R & 0x7F) + 1) & 0x7F);
        break;
    }

    size_t cycles = 0;

    if (_is_nmi) {
        /*
         * Process NMI.
         */
        _IFF2 = _IFF1;
        _IFF1 = false;
        push_addr(_regs.PC);
        _regs.PC = vNMI;
        _regs.memptr = vNMI;
        _log.debug("Detected NMI interrupt, PC: $%04X\n", _regs.PC);
        cycles += 17;  /* CALL vNMI */

    } else {
        /*
         * Process INT.
         */
        _IFF1 = false;
        _IFF2 = false;

        uint8_t vec = _mmap->databus();
        iorq_pin(false);

        switch (_imode) {
        case IMODE_0:
            /*
             * The device provides the 8-bits instruction.
             */
            _log.debug("Detected INT interrupt, mode: 0, opcode: $%02X \"%s\"\n", vec, instr_set[vec].format);
            cycles = execute(vec, true);
            if (cycles == 0) {
                return 0;
            }
            break;

        case IMODE_1:
            /*
             * ISR at vINT ($0038).
             */
            push_addr(_regs.PC);
            _regs.PC = vINT;
            _regs.memptr = vINT;
            _log.debug("Detected INT interrupt, mode: 1, PC: $%04X\n", _regs.PC);
            cycles += 17;  /* CALL vINT */
            break;

        case IMODE_2:
        default:
            /*
             * The interrupting device provides an 8-bits vector used to address an ISR table.
             */
            addr_t isr_table = static_cast<addr_t>(_regs.I) << 8;
            addr_t isr_ptr = isr_table | vec;
            push_addr(_regs.PC);
            _regs.PC = read_addr(isr_ptr);
            _regs.memptr = _regs.PC;
            _log.debug("Detected INT interrupt, mode: 2, ISR table: $%04X, vector: $%02X, ISR: $%04X\n",
                isr_table, vec, _regs.PC);

            /*
             * 7 cycles to fetch the lower eight bits from the interrupting device,
             * 6 cycles to save the program counter, and
             * 6 cycles to obtain the jump address.
             */
            cycles += 7 + 6 + 6;
            break;
        }
    }

    /*
     * Start the refresh cycle.
     */
    rfsh_pin(true);
    addr_t rfsh_addr = (static_cast<addr_t>(_regs.I) << 8) | _regs.R;
    read(rfsh_addr);    /* Force rfsh_addr on the address bus */

    _tx = Cycle::T1;
    _is_nmi = false;
    _is_int = false;

    /*
     * The cycles variable include T1 and T2, both already consumed by the system clock.
     */
    return (cycles - 2);
}

size_t Z80::execute(uint8_t opcode, bool forced)
{
    std::string line{};
    auto& ins = instr_set[opcode];

    if (_log.is_debug()) {
        if (forced) {
            line = "Forced instruction: "s + ins.format;
        } else {
            addr_t addr = _regs.PC;
            line = disass(addr);
        }
    }

    size_t cycles = execute(&ins, opcode, forced);

    if (_log.is_debug()) {
        std::ostringstream msg{};
        msg << std::setw(35) << std::left << line << "  cycles=" << cycles << std::endl
            << status() << std::endl;
        _log.debug(msg.str());
    }

    return cycles;
}

size_t Z80::execute(const Z80::Instruction* ins, uint8_t opcode, bool forced)
{
    size_t cycles{};
    addr_t arg{};

    /*
     * If forced is false:
     *   At input, the PC regsiter must point to the specified opcode;
     *   At exit, the PC register points to the next instruction.
     *
     * If forced is true:
     *   If the opcode is a single-byte instruction the PC register is left unchanged;
     *   If the opcode is a multi-byte instruction the PC register is changed and the behaviour
     *   of the execution is unpredictable.
     *
     * Forced opcodes MUST be single-byte instructions.
     */
    switch (ins->type) {
    case ArgType::A8:
        arg = static_cast<addr_t>(read(++_regs.PC));
        ++_regs.PC;
        break;

    case ArgType::A16:
        arg = read(++_regs.PC) | (static_cast<addr_t>(read(++_regs.PC)) << 8);
        ++_regs.PC;
        break;

    case ArgType::None:
        if (!forced) {
            ++_regs.PC;
        }
        break;

    case ArgType::Bit:
        opcode = read(++_regs.PC);
        ins = &bit_instr_set[opcode];
        return execute(ins, opcode);

    case ArgType::IX:
        opcode = read(++_regs.PC);
        ins = &ix_instr_set[opcode];
        return execute(ins, opcode);

    case ArgType::IY:
        opcode = read(++_regs.PC);
        ins = &iy_instr_set[opcode];
        return execute(ins, opcode);

    case ArgType::IXBit:
        opcode = read(_regs.PC + 2);
        ins = &ix_bit_instr_set[opcode];
        cycles = execute(ins, opcode);
        ++_regs.PC;
        return cycles;

    case ArgType::IYBit:
        opcode = read(_regs.PC + 2);
        ins = &iy_bit_instr_set[opcode];
        cycles = execute(ins, opcode);
        ++_regs.PC;
        return cycles;

    case ArgType::MI:
        opcode = read(++_regs.PC);
        ins = &mi_instr_set[opcode];
        return execute(ins, opcode);

    default:
        //FIXME
        log.error("Invalid instruction $%02X at $%04X, argtype %d\n", opcode, _regs.PC, ins->type);
        ebreak();
        return 4;
    }

    int c = ins->fn(*this, opcode, arg);
    c &= 0x0000FFFF;
    return (c == 0 ? ins->cycles : c);
}

inline size_t Z80::tick()
{
    if (_reset_pin) {
        /*
         * Do nothing until the /RESET pin is disabled.
         */
        return 1;
    }

    auto cycles = ((_is_int || _is_nmi) ? m1_cycle_interrupt() : m1_cycle());
    return cycles;
}

size_t Z80::tick(const Clock& clk)
{
    if (_break && !_monitor) {
        /*
         * Break hot-key but monitor not active.
         */
        log.debug("System halt requested from breakpoint\n");
        return Clockable::HALT;
    }

    /*
     * Break hot-key or breakpoint from monitor.
     */
    if (_monitor && _tx == Cycle::T1 && (_break || _monitor->is_breakpoint(_regs.PC))) {
        addr_t pc{};
        do {
            _break = false;
            pc = _regs.PC;
            if (!_monitor->run()) {
                log.debug("System halt requested from monitor\n");
                return Clockable::HALT;
            }
        } while (pc != _regs.PC && _monitor->is_breakpoint(_regs.PC));
    }

    /*
     * System breakpoints (from some part of the emulator).
     */
    auto bp = _breakpoints.find(_regs.PC);
    if (bp != _breakpoints.end()) {
        auto& fn = bp->second.first;
        auto* arg = bp->second.second;
        fn(*this, arg);
    }

    size_t cycles = tick();
    return (cycles == 0 ? Clockable::HALT : cycles);
}

void Z80::disass(std::ostream& os, addr_t start, size_t count, bool show_pc)
{
    for (addr_t addr = start; count; --count) {
        const std::string& line = disass(addr, show_pc);
        os << line << std::endl;
    }
}

std::string Z80::disass(addr_t& addr, bool show_pc)
{
    /*
     * Output format:
     *  8000: 00 00        LD  A(HL)
     *  8002: 00 00 00 00  AND A
     *  8005: 00           RET
     *
     *  0         1         2         3
     *  012345678901234567890123456789012345
     */
    constexpr static const size_t HEX_SIZE = 20;
    constexpr static const size_t MNE_SIZE = 20;

    addr_t faddr = addr;
    auto* iset = &instr_set;

    const Instruction* ins{};
    uint8_t oplo{}, ophi{};
    uint8_t opcode{};
    bool has_oplo{};

    /*
     * Print address.
     */
    std::ostringstream hex{};
    hex << utils::to_string(addr) << ":";

    /*
     * Print opcode[s].
     */
    bool multibyte{true};
    do {
        opcode = peek(addr++);
        hex << " " << utils::to_string(opcode);

        ins = &iset->operator[](opcode);
        switch (ins->type) {
        case ArgType::MI:
            iset = &mi_instr_set;
            break;
        case ArgType::Bit:
            iset = &bit_instr_set;
            break;
        case ArgType::IX:
            iset = &ix_instr_set;
            break;
        case ArgType::IY:
            iset = &iy_instr_set;
            break;
        case ArgType::IXBit:
            oplo = peek(addr++);
            has_oplo = true;
            hex << " " << utils::to_string(oplo);
            iset = &ix_bit_instr_set;
            break;
        case ArgType::IYBit:
            oplo = peek(addr++);
            has_oplo = true;
            hex << " " << utils::to_string(oplo);
            iset = &iy_bit_instr_set;
            break;
        default:
            multibyte = false;
        }
    } while (multibyte);

    /*
     * Get the instruction operators based on the format string.
     */
    std::string format{ins->format};
    auto pos = format.find_first_of("*^%+");
    if (pos != std::string::npos) {
        /*
         * '^', '%', '*', '+' is present in the format string: Find the operands.
         */
        char v = format[pos];

        /*
         * First byte of the operand.
         */
        std::ostringstream displ{};
        std::ostringstream ops{};

        addr_t operand{};
        char sign{};

        if (!has_oplo) {
            oplo = peek(addr++);
            hex << " " << utils::to_string(oplo);
        }

        switch (v) {
        case '*':
        case '^':
            /*
             * If argument is an 8 bits value it must be disassembled as $00
             * If argument is a 16 bits value it must be disassembled as $0000
             */
            switch (ins->type) {
            case ArgType::A8:
                ops << utils::to_string(oplo);
                format = format.replace(pos, 1, ops.str());
                break;

            case ArgType::A16:
                ophi = peek(addr++);
                operand = (static_cast<addr_t>(ophi) << 8) | oplo;
                hex << " " << utils::to_string(ophi);
                ops << utils::to_string(operand);
                format = format.replace(pos, 1, ops.str());
                break;

            default:
                /* Internal error */
                log.fatal("disass: Invalid format string: \"%s\", opcode: $%02X, disassembled: %s\n",
                    format.c_str(), opcode, hex.str().c_str());
                /* NOTREACHED */
            }
            break;

        case '%':
            /*
             * If argument is an 8 bits value it must be disassembled as +/- $hex
             * If argument is a 16 bits value it must be disassembled as +/- $hex and $00
             * (that is, the next byte is printed as sign+hexadecimal, and the second byte as hexadecimal).
             */
            if (oplo & 0x80) {
                oplo -= oplo;
                sign = '-';
            } else {
                sign = '+';
            }

            switch (ins->type) {
            case ArgType::A8:
                displ << sign << "$" << utils::to_string(oplo);
                format.replace(pos, 1, displ.str());
                break;
            case ArgType::A16:
                displ << sign << "$" << utils::to_string(oplo);
                format.replace(pos, 1, displ.str());
                pos = format.find_first_of("*");
                if (pos == std::string::npos) {
                    /* Internal error */
                    log.fatal("disass: A16 instruction, missing 8 bits argument: Invalid format string: \"%s\","
                        "opcode: $%02X, disassembled: %s\n", format.c_str(), opcode, hex.str().c_str());
                    /* NOTREACHED */
                }
                ophi = peek(addr++);
                ops << utils::to_string(ophi);
                format = format.replace(pos, 1, ops.str());
                break;
            default:
                /* Internal error */
                log.fatal("disass: Invalid format string: \"%s\", opcode: $%02X, disassembled: %s\n",
                    format.c_str(), opcode, hex.str().c_str());
                /* NOTREACHED */
            }
            break;

        case '+':
            /*
             * The argument is an 8 bits relative address: The disassembled string must show the absolute address.
             */
            operand = static_cast<int8_t>(oplo) + addr;
            ops << utils::to_string(operand);
            format = format.replace(pos, 1, ops.str());
            break;
        }
    }

    /*
     * Build the string for the disassembled instruction.
     */
    std::ostringstream line{};
    line << std::setw(HEX_SIZE) << std::left << hex.str()
         << std::setw(MNE_SIZE) << std::left << format;

    if (show_pc && _regs.PC >= faddr && _regs.PC < addr) {
        line << "<";
    }

    return line.str();
}

addr_t Z80::read_addr(size_t addr)
{
    uint8_t lo = read(addr);
    uint8_t hi = read(addr + 1);

    return (static_cast<addr_t>(hi) << 8 | lo);
}

void Z80::write_addr(addr_t addr, addr_t data)
{
    uint8_t lo = static_cast<uint8_t>(data & 0xFF);
    uint8_t hi = static_cast<uint8_t>(data >> 8);

    write(addr, lo);
    write(addr + 1, hi);
}

uint8_t Z80::read(addr_t addr, Device::ReadMode mode)
{
    return _mmap->read(addr, mode);
}

void Z80::write(addr_t addr, uint8_t data)
{
    _mmap->write(addr, data);
}

void Z80::ebreak()
{
    _break = true;
}

void Z80::bpadd(addr_t addr, const breakpoint_cb_t& cb, void* arg)
{
    _breakpoints[addr] = {cb, arg};
}

void Z80::bpdel(addr_t addr)
{
    _breakpoints.erase(addr);
}

const Z80::Registers &Z80::regs() const
{
    return _regs;
}

std::string Z80::status() const
{
    std::ostringstream os{};

    os << this->_regs.to_string() << std::endl
       << "  IFF1=" << this->_IFF1
       << " IFF2="  << this->_IFF2
       << " MI="    << this->_imode
       << " HALT="  << this->_halted;

    return os.str();
}

}
}
