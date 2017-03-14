#include "cpu.h"

using namespace libdmg;

const CPU::Instruction CPU::INSTRUCTION_MAP[] =
{
	/* 0x00 */{ "NOP", 1, 4, &CPU::nop },
	/* 0x01 */{ "LD BC,0x%04X", 3, 12, &CPU::load_constant_16bit },
	/* 0x02 */{ "LD (BC),A", 1, 8, &CPU::load_accumulator_to_memory },
	/* 0x03 */{ "INC BC", 1, 8, &CPU::alu_inc_16bit },
	/* 0x04 */{ "INC B", 1, 4, &CPU::alu_inc },
	/* 0x05 */{ "DEC B", 1, 4, &CPU::alu_dec },
	/* 0x06 */{ "LD B,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x07 */{ "<Unkown>", 1, 4, NULL },
	/* 0x08 */{ "<Unkown>", 3, 20, NULL },
	/* 0x09 */{ "<Unkown>", 1, 8, NULL },
	/* 0x0A */{ "<Unkown>", 1, 8, &CPU::load_memory_to_accumulator },
	/* 0x0B */{ "DEC BC", 1, 8, &CPU::alu_dec_16bit },
	/* 0x0C */{ "INC C", 1, 4, &CPU::alu_inc },
	/* 0x0D */{ "DEC C", 1, 4, &CPU::alu_dec },
	/* 0x0E */{ "LD C,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x0F */{ "<Unkown>", 1, 4, NULL },

	/* 0x10 */{ "<Unkown>", 2, 4, NULL },
	/* 0x11 */{ "LD DE,0x%04X", 3, 12, &CPU::load_constant_16bit },
	/* 0x12 */{ "LD (DE),A", 1, 8, &CPU::load_accumulator_to_memory },
	/* 0x13 */{ "INC DE", 1, 8, &CPU::alu_inc_16bit },
	/* 0x14 */{ "INC D", 1, 4, &CPU::alu_inc },
	/* 0x15 */{ "DEC D", 1, 4, &CPU::alu_dec },
	/* 0x16 */{ "LD D,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x17 */{ "<Unkown>", 1, 4, NULL },
	/* 0x18 */{ "JR 0x%02X", 2, 12, &CPU::jump_to_offset },
	/* 0x19 */{ "<Unkown>", 1, 8, NULL },
	/* 0x1A */{ "<Unkown>", 1, 8, &CPU::load_memory_to_accumulator },
	/* 0x1B */{ "DEC DE", 1, 8, &CPU::alu_dec_16bit },
	/* 0x1C */{ "INC E", 1, 4, &CPU::alu_inc },
	/* 0x1D */{ "DEC E", 1, 4, &CPU::alu_dec },
	/* 0x1E */{ "LD E,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x1F */{ "<Unkown>", 1, 4, NULL },

	/* 0x20 */{ "JR NZ,0x%02X", 2, 8, &CPU::jump_to_offset_conditional },
	/* 0x21 */{ "LD HL,0x%04X", 3, 12, &CPU::load_constant_16bit },
	/* 0x22 */{ "LD (HL+),A", 1, 8, &CPU::load_accumulator_to_memory },
	/* 0x23 */{ "INC HL", 1, 8, &CPU::alu_inc_16bit },
	/* 0x24 */{ "INC H", 1, 4, &CPU::alu_inc },
	/* 0x25 */{ "DEC H", 1, 4, &CPU::alu_dec },
	/* 0x26 */{ "LD H,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x27 */{ "DAA", 1, 4, &CPU::encode_bcd },
	/* 0x28 */{ "JR Z,0x%02X", 2, 8, &CPU::jump_to_offset_conditional },
	/* 0x29 */{ "<Unkown>", 1, 8, NULL },
	/* 0x2A */{ "<Unkown>", 1, 8, &CPU::load_memory_to_accumulator },
	/* 0x2B */{ "DEC HL", 1, 8, &CPU::alu_dec_16bit },
	/* 0x2C */{ "INC L", 1, 4, &CPU::alu_inc },
	/* 0x2D */{ "DEC L", 1, 4, &CPU::alu_dec },
	/* 0x2E */{ "LD L,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x2F */{ "<Unkown>", 1, 4, NULL },

	/* 0x30 */{ "JR NC,0x%02X", 2, 8, &CPU::jump_to_offset_conditional },
	/* 0x31 */{ "LD SP,0x%04X", 3, 12, &CPU::load_constant_16bit },
	/* 0x32 */{ "LD (HL-),A", 1, 8, &CPU::load_accumulator_to_memory },
	/* 0x33 */{ "INC SP", 1, 8, &CPU::alu_inc_16bit },
	/* 0x34 */{ "INC (HL)", 1, 12, &CPU::alu_inc },
	/* 0x35 */{ "DEC (HL)", 1, 12, &CPU::alu_dec },
	/* 0x36 */{ "LD (HL),0x%02X", 2, 8, &CPU::load_constant },
	/* 0x37 */{ "<Unkown>", 1, 4, NULL },
	/* 0x38 */{ "JR C,0x%02X", 2, 8, &CPU::jump_to_offset_conditional },
	/* 0x39 */{ "<Unkown>", 1, 8, NULL },
	/* 0x3A */{ "<Unkown>", 1, 8, &CPU::load_memory_to_accumulator },
	/* 0x3B */{ "DEC SP", 1, 8, &CPU::alu_dec_16bit },
	/* 0x3C */{ "INC A", 1, 4, &CPU::alu_inc },
	/* 0x3D */{ "DEC A", 1, 4, &CPU::alu_dec },
	/* 0x3E */{ "LD A,0x%02X", 2, 8, &CPU::load_constant },
	/* 0x3F */{ "<Unkown>", 1, 4, NULL },

	/* 0x40 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x41 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x42 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x43 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x44 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x45 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x46 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x47 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x48 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x49 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x4A */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x4B */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x4C */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x4D */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x4E */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x4F */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },

	/* 0x50 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x51 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x52 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x53 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x54 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x55 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x56 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x57 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x58 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x59 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x5A */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x5B */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x5C */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x5D */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x5E */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x5F */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },

	/* 0x60 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x61 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x62 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x63 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x64 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x65 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x66 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x67 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x68 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x69 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x6A */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x6B */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x6C */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x6D */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x6E */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x6F */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },

	/* 0x70 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x71 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x72 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x73 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x74 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x75 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x76 */{ "HALT", 1, 4, NULL },
	/* 0x77 */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x78 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x79 */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x7A */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x7B */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x7C */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x7D */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },
	/* 0x7E */{ "LD ?,?", 1, 8, &CPU::load_memory_to_memory },
	/* 0x7F */{ "LD ?,?", 1, 4, &CPU::load_memory_to_memory },

	/* 0x80 */{ "ADD B", 1, 4, &CPU::alu_add },
	/* 0x81 */{ "ADD C", 1, 4, &CPU::alu_add },
	/* 0x82 */{ "ADD D", 1, 4, &CPU::alu_add },
	/* 0x83 */{ "ADD E", 1, 4, &CPU::alu_add },
	/* 0x84 */{ "ADD H", 1, 4, &CPU::alu_add },
	/* 0x85 */{ "ADD L", 1, 4, &CPU::alu_add },
	/* 0x86 */{ "ADD (HL)", 1, 8, &CPU::alu_add },
	/* 0x87 */{ "ADD A", 1, 4, &CPU::alu_add },
	/* 0x88 */{ "ADC B", 1, 4, &CPU::alu_adc },
	/* 0x89 */{ "ADC C", 1, 4, &CPU::alu_adc },
	/* 0x8A */{ "ADC D", 1, 4, &CPU::alu_adc },
	/* 0x8B */{ "ADC E", 1, 4, &CPU::alu_adc },
	/* 0x8C */{ "ADC H", 1, 4, &CPU::alu_adc },
	/* 0x8D */{ "ADC L", 1, 4, &CPU::alu_adc },
	/* 0x8E */{ "ADC (HL)", 1, 8, &CPU::alu_adc },
	/* 0x8F */{ "ADC A", 1, 4, &CPU::alu_adc },

	/* 0x90 */{ "SUB B", 1, 4, &CPU::alu_sub },
	/* 0x91 */{ "SUB C", 1, 4, &CPU::alu_sub },
	/* 0x92 */{ "SUB D", 1, 4, &CPU::alu_sub },
	/* 0x93 */{ "SUB E", 1, 4, &CPU::alu_sub },
	/* 0x94 */{ "SUB H", 1, 4, &CPU::alu_sub },
	/* 0x95 */{ "SUB L", 1, 4, &CPU::alu_sub },
	/* 0x96 */{ "SUB (HL)", 1, 8, &CPU::alu_sub },
	/* 0x97 */{ "SUB A", 1, 4, &CPU::alu_sub },
	/* 0x98 */{ "SBC B", 1, 4, &CPU::alu_sbc },
	/* 0x99 */{ "SBC C", 1, 4, &CPU::alu_sbc },
	/* 0x9A */{ "SBC D", 1, 4, &CPU::alu_sbc },
	/* 0x9B */{ "SBC E", 1, 4, &CPU::alu_sbc },
	/* 0x9C */{ "SBC H", 1, 4, &CPU::alu_sbc },
	/* 0x9D */{ "SBC L", 1, 4, &CPU::alu_sbc },
	/* 0x9E */{ "SBC (HL)", 1, 8, &CPU::alu_sbc },
	/* 0x9F */{ "SBC A", 1, 4, &CPU::alu_sbc },

	/* 0xA0 */{ "AND B", 1, 4, &CPU::alu_and },
	/* 0xA1 */{ "AND C", 1, 4, &CPU::alu_and },
	/* 0xA2 */{ "AND D", 1, 4, &CPU::alu_and },
	/* 0xA3 */{ "AND E", 1, 4, &CPU::alu_and },
	/* 0xA4 */{ "AND H", 1, 4, &CPU::alu_and },
	/* 0xA5 */{ "AND L", 1, 4, &CPU::alu_and },
	/* 0xA6 */{ "AND (HL)", 1, 8, &CPU::alu_and },
	/* 0xA7 */{ "AND A", 1, 4, &CPU::alu_and },
	/* 0xA8 */{ "XOR B", 1, 4, &CPU::alu_xor },
	/* 0xA9 */{ "XOR C", 1, 4, &CPU::alu_xor },
	/* 0xAA */{ "XOR D", 1, 4, &CPU::alu_xor },
	/* 0xAB */{ "XOR E", 1, 4, &CPU::alu_xor },
	/* 0xAC */{ "XOR H", 1, 4, &CPU::alu_xor },
	/* 0xAD */{ "XOR L", 1, 4, &CPU::alu_xor },
	/* 0xAE */{ "XOR (HL)", 1, 8, &CPU::alu_xor },
	/* 0xAF */{ "XOR A", 1, 4, &CPU::alu_xor },

	/* 0xB0 */{ "OR B", 1, 4, &CPU::alu_or },
	/* 0xB1 */{ "OR C", 1, 4, &CPU::alu_or },
	/* 0xB2 */{ "OR D", 1, 4, &CPU::alu_or },
	/* 0xB3 */{ "OR E", 1, 4, &CPU::alu_or },
	/* 0xB4 */{ "OR H", 1, 4, &CPU::alu_or },
	/* 0xB5 */{ "OR L", 1, 4, &CPU::alu_or },
	/* 0xB6 */{ "OR (HL)", 1, 8, &CPU::alu_or },
	/* 0xB7 */{ "OR A", 1, 4, &CPU::alu_or },
	/* 0xA8 */{ "CP B", 1, 4, &CPU::alu_cmp },
	/* 0xA9 */{ "CP C", 1, 4, &CPU::alu_cmp },
	/* 0xAA */{ "CP D", 1, 4, &CPU::alu_cmp },
	/* 0xAB */{ "CP E", 1, 4, &CPU::alu_cmp },
	/* 0xAC */{ "CP H", 1, 4, &CPU::alu_cmp },
	/* 0xAD */{ "CP L", 1, 4, &CPU::alu_cmp },
	/* 0xAE */{ "CP (HL)", 1, 8, &CPU::alu_cmp },
	/* 0xAF */{ "CP A", 1, 4, &CPU::alu_cmp },

	/* 0xC0 */{ "RET NZ", 1, 8, &CPU::return_conditional },
	/* 0xC1 */{ "<Unkown>", 1, 12, NULL },
	/* 0xC2 */{ "<Unkown>", 3, 12, NULL },
	/* 0xC3 */{ "JP 0x%04X", 3, 16, &CPU::jump },
	/* 0xC4 */{ "<Unkown>", 3, 12, NULL },
	/* 0xC5 */{ "<Unkown>", 1, 16, NULL },
	/* 0xC6 */{ "ADD A,0x%02X", 2, 8, &CPU::alu_add },
	/* 0xC7 */{ "RST 00H", 1, 16, &CPU::restart },
	/* 0xC8 */{ "RET Z", 1, 8, &CPU::return_conditional },
	/* 0xC9 */{ "RET", 1, 16, &CPU::return_default },
	/* 0xCA */{ "<Unkown>", 3, 12, NULL },
	/* 0xCB */{ "Prefix CB", 1, 4, NULL },
	/* 0xCC */{ "<Unkown>", 3, 12, NULL },
	/* 0xCD */{ "CALL 0x%04X", 3, 24, &CPU::call },
	/* 0xCE */{ "ADC A,0x%02X", 2, 8, &CPU::alu_adc },
	/* 0xCF */{ "RST 08H", 1, 16, &CPU::restart },

	/* 0xD0 */{ "RET NC", 1, 8, &CPU::return_conditional },
	/* 0xD1 */{ "<Unkown>", 1, 12, NULL },
	/* 0xD2 */{ "<Unkown>", 3, 12, NULL },
	/* 0xD3 */{ "N/I", 0, 0, NULL },
	/* 0xD4 */{ "<Unkown>", 3, 12, NULL },
	/* 0xD5 */{ "<Unkown>", 1, 16, NULL },
	/* 0xD6 */{ "SUB 0x%02X", 2, 8, &CPU::alu_sub },
	/* 0xD7 */{ "RST 10H", 1, 16, &CPU::restart },
	/* 0xD8 */{ "RET C", 1, 8, &CPU::return_conditional },
	/* 0xD9 */{ "RETI", 1, 16, &CPU::return_enable_interrupts },
	/* 0xDA */{ "<Unkown>", 3, 12, NULL },
	/* 0xDB */{ "N/I", 0, 0, NULL },
	/* 0xDC */{ "<Unkown>", 3, 12, NULL },
	/* 0xDD */{ "N/I", 0, 0, NULL },
	/* 0xDE */{ "SBC A,0x%02X", 2, 8, &CPU::alu_sbc },
	/* 0xDF */{ "RST 18H", 1, 16, &CPU::restart },

	/* 0xE0 */{ "LDH (0xFF%02X),A", 2, 12, &CPU::load_accumulator_to_constant_io_register },
	/* 0xE1 */{ "<Unkown>", 1, 4, NULL },
	/* 0xE2 */{ "LD (0xFF00+C),A", 2, 8, &CPU::load_accumulator_to_c_plus_io_register },
	/* 0xE3 */{ "N/I", 0, 0, NULL },
	/* 0xE4 */{ "N/I", 0, 0, NULL },
	/* 0xE5 */{ "<Unkown>", 1, 16, NULL },
	/* 0xE6 */{ "AND 0x%02X", 2, 8, &CPU::alu_and },
	/* 0xE7 */{ "RST 20H", 1, 16, &CPU::restart },
	/* 0xE8 */{ "<Unkown>", 2, 16, NULL },
	/* 0xE9 */{ "<Unkown>", 1, 4, NULL },
	/* 0xEA */{ "LD (0x%04X),A", 3, 16, &CPU::load_accumulator_to_memory_16bit },
	/* 0xEB */{ "N/I", 0, 0, NULL },
	/* 0xEC */{ "N/I", 0, 0, NULL },
	/* 0xED */{ "N/I", 0, 0, NULL },
	/* 0xEE */{ "XOR 0x%02X", 2, 8, &CPU::alu_xor },
	/* 0xEF */{ "RST 28H", 1, 16, &CPU::restart },

	/* 0xF0 */{ "LDH A,(0xFF%02X)", 2, 12, &CPU::load_constant_io_register_to_accumulator },
	/* 0xF1 */{ "<Unkown>", 1, 12, NULL },
	/* 0xF2 */{ "LD A,(0xFF00+C)", 2, 8, &CPU::load_c_plus_io_register_to_accumulator },
	/* 0xF3 */{ "DI", 1, 4, &CPU::disable_interrupts },
	/* 0xF4 */{ "N/I", 0, 0, NULL },
	/* 0xF5 */{ "<Unkown>", 1, 16, NULL },
	/* 0xF6 */{ "OR 0x%2X", 2, 8, &CPU::alu_or },
	/* 0xF7 */{ "RST 30H", 1, 16, &CPU::restart },
	/* 0xF8 */{ "LD HL,SP+0x%02X", 2, 12, &CPU::load_sp_plus_constant_to_hl },
	/* 0xF9 */{ "LD SP,HL", 1, 8, &CPU::load_hl_to_sp },
	/* 0xFA */{ "LD A,(0x%04X)", 3, 16, &CPU::load_memory_to_accumulator_16bit },
	/* 0xFB */{ "EI", 1, 4, &CPU::enable_interupts },
	/* 0xFC */{ "N/I", 0, 0, NULL },
	/* 0xFD */{ "N/I", 0, 0, NULL },
	/* 0xFE */{ "CP 0x%02X", 2, 8, &CPU::alu_cmp },
	/* 0xFF */{ "RST 38H", 1, 16, &CPU::restart },
};
