// Package z80 implements a Z80 CPU emulator with support for all documented
// and undocumented opcodes, flags, and registers.
package z80

import "fmt"

// ExecuteOpcode executes a base (unprefixed) opcode and returns the number of T-states used
func (cpu *CPU) ExecuteOpcode(opcode byte) int {
	switch opcode {
	// 8-bit load group
	case 0x00: // NOP
		return 4
	case 0x01: // LD BC, nn
		cpu.SetBC(cpu.ReadImmediateWord())
		return 10
	case 0x02: // LD (BC), A
		cpu.Memory.WriteByte(cpu.GetBC(), cpu.A)
		cpu.MEMPTR = (uint16(cpu.A) << 8) | (uint16(cpu.GetBC()+1) & 0xff)
		return 7
	case 0x03: // INC BC
		cpu.SetBC(cpu.GetBC() + 1)
		return 6
	case 0x04: // INC B
		cpu.B = cpu.inc8(cpu.B)
		return 4
	case 0x05: // DEC B
		cpu.B = cpu.dec8(cpu.B)
		return 4
	case 0x06: // LD B, n
		cpu.B = cpu.ReadImmediateByte()
		return 7
	case 0x07: // RLCA
		cpu.rlca()
		return 4
	case 0x08: // EX AF, AF'
		temp := cpu.GetAF()
		cpu.SetAF(cpu.GetAF_())
		cpu.SetAF_(temp)
		return 4
	case 0x09: // ADD HL, BC
		result := cpu.add16(cpu.GetHL(), cpu.GetBC())
		cpu.MEMPTR = cpu.GetHL() + 1
		cpu.SetHL(result)
		return 11
	case 0x0A: // LD A, (BC)
		cpu.A = cpu.Memory.ReadByte(cpu.GetBC())
		cpu.MEMPTR = cpu.GetBC() + 1
		return 7
	case 0x0B: // DEC BC
		cpu.SetBC(cpu.GetBC() - 1)
		return 6
	case 0x0C: // INC C
		cpu.C = cpu.inc8(cpu.C)
		return 4
	case 0x0D: // DEC C
		cpu.C = cpu.dec8(cpu.C)
		return 4
	case 0x0E: // LD C, n
		cpu.C = cpu.ReadImmediateByte()
		return 7
	case 0x0F: // RRCA
		cpu.rrca()
		return 4
	case 0x10: // DJNZ e
		cpu.B--
		if cpu.B != 0 {
			offset := cpu.ReadDisplacement()
			cpu.PC = uint16(int32(cpu.PC) + int32(offset))
			return 13
		}
		cpu.PC++ // Skip the offset byte
		return 8
	case 0x11: // LD DE, nn
		cpu.SetDE(cpu.ReadImmediateWord())
		return 10
	case 0x12: // LD (DE), A
		cpu.Memory.WriteByte(cpu.GetDE(), cpu.A)
		cpu.MEMPTR = (uint16(cpu.A) << 8) | (uint16(cpu.GetDE()+1) & 0xff)
		return 7
	case 0x13: // INC DE
		cpu.SetDE(cpu.GetDE() + 1)
		return 6
	case 0x14: // INC D
		cpu.D = cpu.inc8(cpu.D)
		return 4
	case 0x15: // DEC D
		cpu.D = cpu.dec8(cpu.D)
		return 4
	case 0x16: // LD D, n
		cpu.D = cpu.ReadImmediateByte()
		return 7
	case 0x17: // RLA
		cpu.rla()
		return 4
	case 0x18: // JR e
		offset := cpu.ReadDisplacement()
		cpu.MEMPTR = cpu.PC + uint16(int32(offset))
		cpu.PC = uint16(int32(cpu.PC) + int32(offset))
		return 12
	case 0x19: // ADD HL, DE
		result := cpu.add16(cpu.GetHL(), cpu.GetDE())
		cpu.MEMPTR = cpu.GetHL() + 1
		cpu.SetHL(result)
		return 11
	case 0x1A: // LD A, (DE)
		cpu.A = cpu.Memory.ReadByte(cpu.GetDE())
		cpu.MEMPTR = cpu.GetDE() + 1
		return 7
	case 0x1B: // DEC DE
		cpu.SetDE(cpu.GetDE() - 1)
		return 6
	case 0x1C: // INC E
		cpu.E = cpu.inc8(cpu.E)
		return 4
	case 0x1D: // DEC E
		cpu.E = cpu.dec8(cpu.E)
		return 4
	case 0x1E: // LD E, n
		cpu.E = cpu.ReadImmediateByte()
		return 7
	case 0x1F: // RRA
		cpu.rra()
		return 4
	case 0x20: // JR NZ, e
		if !cpu.GetFlag(FLAG_Z) {
			offset := cpu.ReadDisplacement()
			cpu.MEMPTR = cpu.PC + uint16(int32(offset))
			cpu.PC = uint16(int32(cpu.PC) + int32(offset))
			return 12
		}
		cpu.PC++ // Skip the offset byte
		return 7
	case 0x21: // LD HL, nn
		cpu.SetHL(cpu.ReadImmediateWord())
		return 10
	case 0x22: // LD (nn), HL
		addr := cpu.ReadImmediateWord()
		cpu.Memory.WriteWord(addr, cpu.GetHL())
		cpu.MEMPTR = addr + 1
		return 16
	case 0x23: // INC HL
		cpu.SetHL(cpu.GetHL() + 1)
		return 6
	case 0x24: // INC H
		cpu.H = cpu.inc8(cpu.H)
		return 4
	case 0x25: // DEC H
		cpu.H = cpu.dec8(cpu.H)
		return 4
	case 0x26: // LD H, n
		cpu.H = cpu.ReadImmediateByte()
		return 7
	case 0x27: // DAA
		cpu.daa()
		return 4
	case 0x28: // JR Z, e
		if cpu.GetFlag(FLAG_Z) {
			offset := int8(cpu.ReadDisplacement())
			cpu.MEMPTR = uint16(int32(cpu.PC) + int32(offset))
			cpu.PC = uint16(int32(cpu.PC) + int32(offset))
			return 12
		}
		_ = cpu.ReadDisplacement()
		return 7
	case 0x29: // ADD HL, HL
		result := cpu.add16(cpu.GetHL(), cpu.GetHL())
		cpu.MEMPTR = cpu.GetHL() + 1
		cpu.SetHL(result)
		return 11
	case 0x2A: // LD HL, (nn)
		addr := cpu.ReadImmediateWord()
		cpu.SetHL(cpu.Memory.ReadWord(addr))
		cpu.MEMPTR = addr + 1
		return 16
	case 0x2B: // DEC HL
		cpu.SetHL(cpu.GetHL() - 1)
		return 6
	case 0x2C: // INC L
		cpu.L = cpu.inc8(cpu.L)
		return 4
	case 0x2D: // DEC L
		cpu.L = cpu.dec8(cpu.L)
		return 4
	case 0x2E: // LD L, n
		cpu.L = cpu.ReadImmediateByte()
		return 7
	case 0x2F: // CPL
		cpu.cpl()
		return 4
	case 0x30: // JR NC, e
		if !cpu.GetFlag(FLAG_C) {
			offset := cpu.ReadDisplacement()
			cpu.MEMPTR = cpu.PC + uint16(int32(offset))
			cpu.PC = uint16(int32(cpu.PC) + int32(offset))
			return 12
		}
		cpu.PC++ // Skip the offset byte
		return 7
	case 0x31: // LD SP, nn
		cpu.SP = cpu.ReadImmediateWord()
		return 10
	case 0x32: // LD (nn), A
		addr := cpu.ReadImmediateWord()
		cpu.Memory.WriteByte(addr, cpu.A)
		cpu.MEMPTR = (uint16(cpu.A) << 8) | ((addr + 1) & 0xFF)
		return 13
	case 0x33: // INC SP
		cpu.SP++
		return 6
	case 0x34: // INC (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		result := cpu.inc8(value)
		cpu.Memory.WriteByte(cpu.GetHL(), result)
		return 11
	case 0x35: // DEC (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		result := cpu.dec8(value)
		cpu.Memory.WriteByte(cpu.GetHL(), result)
		return 11
	case 0x36: // LD (HL), n
		value := cpu.ReadImmediateByte()
		cpu.Memory.WriteByte(cpu.GetHL(), value)
		return 10
	case 0x37: // SCF
		cpu.scf()
		return 4
	case 0x38: // JR C, e
		if cpu.GetFlag(FLAG_C) {
			offset := cpu.ReadDisplacement()
			cpu.MEMPTR = cpu.PC + uint16(int32(offset))
			cpu.PC = uint16(int32(cpu.PC) + int32(offset))
			return 12
		}
		cpu.PC++ // Skip the offset byte
		return 7
	case 0x39: // ADD HL, SP
		result := cpu.add16(cpu.GetHL(), cpu.SP)
		cpu.MEMPTR = cpu.GetHL() + 1
		cpu.SetHL(result)
		return 11
	case 0x3A: // LD A, (nn)
		addr := cpu.ReadImmediateWord()
		cpu.A = cpu.Memory.ReadByte(addr)
		cpu.MEMPTR = addr + 1
		return 13
	case 0x3B: // DEC SP
		cpu.SP--
		return 6
	case 0x3C: // INC A
		cpu.A = cpu.inc8(cpu.A)
		return 4
	case 0x3D: // DEC A
		cpu.A = cpu.dec8(cpu.A)
		return 4
	case 0x3E: // LD A, n
		cpu.A = cpu.ReadImmediateByte()
		return 7
	case 0x3F: // CCF
		cpu.ccf()
		return 4

	// LD r, r' instructions
	case 0x40: // LD B, B
		return 4
	case 0x41: // LD B, C
		cpu.B = cpu.C
		return 4
	case 0x42: // LD B, D
		cpu.B = cpu.D
		return 4
	case 0x43: // LD B, E
		cpu.B = cpu.E
		return 4
	case 0x44: // LD B, H
		cpu.B = cpu.H
		return 4
	case 0x45: // LD B, L
		cpu.B = cpu.L
		return 4
	case 0x46: // LD B, (HL)
		cpu.B = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x47: // LD B, A
		cpu.B = cpu.A
		return 4
	case 0x48: // LD C, B
		cpu.C = cpu.B
		return 4
	case 0x49: // LD C, C
		return 4
	case 0x4A: // LD C, D
		cpu.C = cpu.D
		return 4
	case 0x4B: // LD C, E
		cpu.C = cpu.E
		return 4
	case 0x4C: // LD C, H
		cpu.C = cpu.H
		return 4
	case 0x4D: // LD C, L
		cpu.C = cpu.L
		return 4
	case 0x4E: // LD C, (HL)
		cpu.C = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x4F: // LD C, A
		cpu.C = cpu.A
		return 4
	case 0x50: // LD D, B
		cpu.D = cpu.B
		return 4
	case 0x51: // LD D, C
		cpu.D = cpu.C
		return 4
	case 0x52: // LD D, D
		return 4
	case 0x53: // LD D, E
		cpu.D = cpu.E
		return 4
	case 0x54: // LD D, H
		cpu.D = cpu.H
		return 4
	case 0x55: // LD D, L
		cpu.D = cpu.L
		return 4
	case 0x56: // LD D, (HL)
		cpu.D = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x57: // LD D, A
		cpu.D = cpu.A
		return 4
	case 0x58: // LD E, B
		cpu.E = cpu.B
		return 4
	case 0x59: // LD E, C
		cpu.E = cpu.C
		return 4
	case 0x5A: // LD E, D
		cpu.E = cpu.D
		return 4
	case 0x5B: // LD E, E
		return 4
	case 0x5C: // LD E, H
		cpu.E = cpu.H
		return 4
	case 0x5D: // LD E, L
		cpu.E = cpu.L
		return 4
	case 0x5E: // LD E, (HL)
		cpu.E = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x5F: // LD E, A
		cpu.E = cpu.A
		return 4
	case 0x60: // LD H, B
		cpu.H = cpu.B
		return 4
	case 0x61: // LD H, C
		cpu.H = cpu.C
		return 4
	case 0x62: // LD H, D
		cpu.H = cpu.D
		return 4
	case 0x63: // LD H, E
		cpu.H = cpu.E
		return 4
	case 0x64: // LD H, H
		return 4
	case 0x65: // LD H, L
		cpu.H = cpu.L
		return 4
	case 0x66: // LD H, (HL)
		cpu.H = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x67: // LD H, A
		cpu.H = cpu.A
		return 4
	case 0x68: // LD L, B
		cpu.L = cpu.B
		return 4
	case 0x69: // LD L, C
		cpu.L = cpu.C
		return 4
	case 0x6A: // LD L, D
		cpu.L = cpu.D
		return 4
	case 0x6B: // LD L, E
		cpu.L = cpu.E
		return 4
	case 0x6C: // LD L, H
		cpu.L = cpu.H
		return 4
	case 0x6D: // LD L, L
		return 4
	case 0x6E: // LD L, (HL)
		cpu.L = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x6F: // LD L, A
		cpu.L = cpu.A
		return 4
	case 0x70: // LD (HL), B
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.B)
		return 7
	case 0x71: // LD (HL), C
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.C)
		return 7
	case 0x72: // LD (HL), D
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.D)
		return 7
	case 0x73: // LD (HL), E
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.E)
		return 7
	case 0x74: // LD (HL), H
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.H)
		return 7
	case 0x75: // LD (HL), L
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.L)
		return 7
	case 0x76: // HALT
		cpu.HALT = true
		cpu.PC--
		return 4
	case 0x77: // LD (HL), A
		cpu.Memory.WriteByte(cpu.GetHL(), cpu.A)
		return 7
	case 0x78: // LD A, B
		cpu.A = cpu.B
		return 4
	case 0x79: // LD A, C
		cpu.A = cpu.C
		return 4
	case 0x7A: // LD A, D
		cpu.A = cpu.D
		return 4
	case 0x7B: // LD A, E
		cpu.A = cpu.E
		return 4
	case 0x7C: // LD A, H
		cpu.A = cpu.H
		return 4
	case 0x7D: // LD A, L
		cpu.A = cpu.L
		return 4
	case 0x7E: // LD A, (HL)
		cpu.A = cpu.Memory.ReadByte(cpu.GetHL())
		return 7
	case 0x7F: // LD A, A
		return 4

	// Arithmetic and logic group
	case 0x80: // ADD A, B
		cpu.add8(cpu.B)
		return 4
	case 0x81: // ADD A, C
		cpu.add8(cpu.C)
		return 4
	case 0x82: // ADD A, D
		cpu.add8(cpu.D)
		return 4
	case 0x83: // ADD A, E
		cpu.add8(cpu.E)
		return 4
	case 0x84: // ADD A, H
		cpu.add8(cpu.H)
		return 4
	case 0x85: // ADD A, L
		cpu.add8(cpu.L)
		return 4
	case 0x86: // ADD A, (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.add8(value)
		return 7
	case 0x87: // ADD A, A
		cpu.add8(cpu.A)
		return 4
	case 0x88: // ADC A, B
		cpu.adc8(cpu.B)
		return 4
	case 0x89: // ADC A, C
		cpu.adc8(cpu.C)
		return 4
	case 0x8A: // ADC A, D
		cpu.adc8(cpu.D)
		return 4
	case 0x8B: // ADC A, E
		cpu.adc8(cpu.E)
		return 4
	case 0x8C: // ADC A, H
		cpu.adc8(cpu.H)
		return 4
	case 0x8D: // ADC A, L
		cpu.adc8(cpu.L)
		return 4
	case 0x8E: // ADC A, (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.adc8(value)
		return 7
	case 0x8F: // ADC A, A
		cpu.adc8(cpu.A)
		return 4
	case 0x90: // SUB B
		cpu.sub8(cpu.B)
		return 4
	case 0x91: // SUB C
		cpu.sub8(cpu.C)
		return 4
	case 0x92: // SUB D
		cpu.sub8(cpu.D)
		return 4
	case 0x93: // SUB E
		cpu.sub8(cpu.E)
		return 4
	case 0x94: // SUB H
		cpu.sub8(cpu.H)
		return 4
	case 0x95: // SUB L
		cpu.sub8(cpu.L)
		return 4
	case 0x96: // SUB (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.sub8(value)
		return 7
	case 0x97: // SUB A
		cpu.sub8(cpu.A)
		return 4
	case 0x98: // SBC A, B
		cpu.sbc8(cpu.B)
		return 4
	case 0x99: // SBC A, C
		cpu.sbc8(cpu.C)
		return 4
	case 0x9A: // SBC A, D
		cpu.sbc8(cpu.D)
		return 4
	case 0x9B: // SBC A, E
		cpu.sbc8(cpu.E)
		return 4
	case 0x9C: // SBC A, H
		cpu.sbc8(cpu.H)
		return 4
	case 0x9D: // SBC A, L
		cpu.sbc8(cpu.L)
		return 4
	case 0x9E: // SBC A, (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.sbc8(value)
		return 7
	case 0x9F: // SBC A, A
		cpu.sbc8(cpu.A)
		return 4
	case 0xA0: // AND B
		cpu.and8(cpu.B)
		return 4
	case 0xA1: // AND C
		cpu.and8(cpu.C)
		return 4
	case 0xA2: // AND D
		cpu.and8(cpu.D)
		return 4
	case 0xA3: // AND E
		cpu.and8(cpu.E)
		return 4
	case 0xA4: // AND H
		cpu.and8(cpu.H)
		return 4
	case 0xA5: // AND L
		cpu.and8(cpu.L)
		return 4
	case 0xA6: // AND (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.and8(value)
		return 7
	case 0xA7: // AND A
		cpu.and8(cpu.A)
		return 4
	case 0xA8: // XOR B
		cpu.xor8(cpu.B)
		return 4
	case 0xA9: // XOR C
		cpu.xor8(cpu.C)
		return 4
	case 0xAA: // XOR D
		cpu.xor8(cpu.D)
		return 4
	case 0xAB: // XOR E
		cpu.xor8(cpu.E)
		return 4
	case 0xAC: // XOR H
		cpu.xor8(cpu.H)
		return 4
	case 0xAD: // XOR L
		cpu.xor8(cpu.L)
		return 4
	case 0xAE: // XOR (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.xor8(value)
		return 7
	case 0xAF: // XOR A
		cpu.xor8(cpu.A)
		return 4
	case 0xB0: // OR B
		cpu.or8(cpu.B)
		return 4
	case 0xB1: // OR C
		cpu.or8(cpu.C)
		return 4
	case 0xB2: // OR D
		cpu.or8(cpu.D)
		return 4
	case 0xB3: // OR E
		cpu.or8(cpu.E)
		return 4
	case 0xB4: // OR H
		cpu.or8(cpu.H)
		return 4
	case 0xB5: // OR L
		cpu.or8(cpu.L)
		return 4
	case 0xB6: // OR (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.or8(value)
		return 7
	case 0xB7: // OR A
		cpu.or8(cpu.A)
		return 4
	case 0xB8: // CP B
		cpu.cp8(cpu.B)
		return 4
	case 0xB9: // CP C
		cpu.cp8(cpu.C)
		return 4
	case 0xBA: // CP D
		cpu.cp8(cpu.D)
		return 4
	case 0xBB: // CP E
		cpu.cp8(cpu.E)
		return 4
	case 0xBC: // CP H
		cpu.cp8(cpu.H)
		return 4
	case 0xBD: // CP L
		cpu.cp8(cpu.L)
		return 4
	case 0xBE: // CP (HL)
		value := cpu.Memory.ReadByte(cpu.GetHL())
		cpu.cp8(value)
		return 7
	case 0xBF: // CP A
		cpu.cp8(cpu.A)
		return 4

	// RET cc instructions
	case 0xC0: // RET NZ
		if !cpu.GetFlag(FLAG_Z) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xC1: // POP BC
		cpu.SetBC(cpu.Pop())
		return 10
	case 0xC2: // JP NZ, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_Z) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xC3: // JP nn
		addr := cpu.ReadImmediateWord()
		cpu.PC = addr
		cpu.MEMPTR = addr
		return 10
	case 0xC4: // CALL NZ, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_Z) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xC5: // PUSH BC
		cpu.Push(cpu.GetBC())
		return 11
	case 0xC6: // ADD A, n
		value := cpu.ReadImmediateByte()
		cpu.add8(value)
		return 7
	case 0xC7: // RST 00H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0000
		cpu.MEMPTR = 0x0000
		return 11
	case 0xC8: // RET Z
		if cpu.GetFlag(FLAG_Z) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xC9: // RET
		cpu.PC = cpu.Pop()
		cpu.MEMPTR = cpu.PC
		return 10
	case 0xCA: // JP Z, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_Z) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xCB: // PREFIX CB
		// This should never be reached as it's handled in ExecuteOneInstruction
		return 0
	case 0xCC: // CALL Z, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_Z) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xCD: // CALL nn
		addr := cpu.ReadImmediateWord()
		cpu.Push(cpu.PC)
		cpu.PC = addr
		cpu.MEMPTR = addr
		return 17
	case 0xCE: // ADC A, n
		value := cpu.ReadImmediateByte()
		cpu.adc8(value)
		return 7
	case 0xCF: // RST 08H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0008
		cpu.MEMPTR = 0x0008
		return 11
	case 0xD0: // RET NC
		if !cpu.GetFlag(FLAG_C) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xD1: // POP DE
		cpu.SetDE(cpu.Pop())
		return 10
	case 0xD2: // JP NC, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_C) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xD3: // OUT (n), A
		n := cpu.ReadImmediateByte()
		port := uint16(n) | (uint16(cpu.A) << 8)
		cpu.IO.WritePort(port, cpu.A)
		cpu.MEMPTR = (uint16(cpu.A) << 8) | uint16((n+1)&0xFF)
		return 11
	case 0xD4: // CALL NC, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_C) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xD5: // PUSH DE
		cpu.Push(cpu.GetDE())
		return 11
	case 0xD6: // SUB n
		value := cpu.ReadImmediateByte()
		cpu.sub8(value)
		return 7
	case 0xD7: // RST 10H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0010
		cpu.MEMPTR = 0x0010
		return 11
	case 0xD8: // RET C
		if cpu.GetFlag(FLAG_C) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xD9: // EXX
		tempBC := cpu.GetBC()
		tempDE := cpu.GetDE()
		tempHL := cpu.GetHL()
		cpu.SetBC(cpu.GetBC_())
		cpu.SetDE(cpu.GetDE_())
		cpu.SetHL(cpu.GetHL_())
		cpu.SetBC_(tempBC)
		cpu.SetDE_(tempDE)
		cpu.SetHL_(tempHL)
		return 4
	case 0xDA: // JP C, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_C) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xDB: // IN A, (n)
		n := cpu.ReadImmediateByte()
		port := uint16(n) | (uint16(cpu.A) << 8)
		cpu.A = cpu.IO.ReadPort(port)
		cpu.MEMPTR = (uint16(cpu.A) << 8) | uint16((n+1)&0xFF)
		return 11
	case 0xDC: // CALL C, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_C) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xDD: // PREFIX DD
		// This should never be reached as it's handled in ExecuteOneInstruction
		return 0
	case 0xDE: // SBC A, n
		value := cpu.ReadImmediateByte()
		cpu.sbc8(value)
		return 7
	case 0xDF: // RST 18H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0018
		cpu.MEMPTR = 0x0018
		return 11
	case 0xE0: // RET PO
		if !cpu.GetFlag(FLAG_PV) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xE1: // POP HL
		cpu.SetHL(cpu.Pop())
		return 10
	case 0xE2: // JP PO, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_PV) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xE3: // EX (SP), HL
		temp := cpu.Memory.ReadWord(cpu.SP)
		cpu.Memory.WriteWord(cpu.SP, cpu.GetHL())
		cpu.SetHL(temp)
		cpu.MEMPTR = temp
		return 19
	case 0xE4: // CALL PO, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_PV) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xE5: // PUSH HL
		cpu.Push(cpu.GetHL())
		return 11
	case 0xE6: // AND n
		value := cpu.ReadImmediateByte()
		cpu.and8(value)
		return 7
	case 0xE7: // RST 20H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0020
		cpu.MEMPTR = 0x0020
		return 11
	case 0xE8: // RET PE
		if cpu.GetFlag(FLAG_PV) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xE9: // JP (HL)
		cpu.PC = cpu.GetHL()
		return 4
	case 0xEA: // JP PE, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_PV) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xEB: // EX DE, HL
		temp := cpu.GetDE()
		cpu.SetDE(cpu.GetHL())
		cpu.SetHL(temp)
		return 4
	case 0xEC: // CALL PE, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_PV) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xED: // PREFIX ED
		// This should never be reached as it's handled in ExecuteOneInstruction
		return 0
	case 0xEE: // XOR n
		value := cpu.ReadImmediateByte()
		cpu.xor8(value)
		return 7
	case 0xEF: // RST 28H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0028
		cpu.MEMPTR = 0x0028
		return 11
	case 0xF0: // RET P
		if !cpu.GetFlag(FLAG_S) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xF1: // POP AF
		cpu.SetAF(cpu.Pop())
		return 10
	case 0xF2: // JP P, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_S) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xF3: // DI
		cpu.IFF1 = false
		cpu.IFF2 = false
		return 4
	case 0xF4: // CALL P, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if !cpu.GetFlag(FLAG_S) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xF5: // PUSH AF
		cpu.Push(cpu.GetAF())
		return 11
	case 0xF6: // OR n
		value := cpu.ReadImmediateByte()
		cpu.or8(value)
		return 7
	case 0xF7: // RST 30H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0030
		cpu.MEMPTR = 0x0030
		return 11
	case 0xF8: // RET M
		if cpu.GetFlag(FLAG_S) {
			cpu.PC = cpu.Pop()
			cpu.MEMPTR = cpu.PC
			return 11
		}
		return 5
	case 0xF9: // LD SP, HL
		cpu.SP = cpu.GetHL()
		return 6
	case 0xFA: // JP M, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_S) {
			cpu.PC = addr
			return 10
		}
		return 10
	case 0xFB: // EI
		cpu.IFF1 = true
		cpu.IFF2 = true
		return 4
	case 0xFC: // CALL M, nn
		addr := cpu.ReadImmediateWord()
		cpu.MEMPTR = addr
		if cpu.GetFlag(FLAG_S) {
			cpu.Push(cpu.PC)
			cpu.PC = addr
			return 17
		}
		return 10
	case 0xFD: // PREFIX FD
		// This should never be reached as it's handled in ExecuteOneInstruction
		return 0
	case 0xFE: // CP n
		value := cpu.ReadImmediateByte()
		cpu.cp8(value)
		return 7
	case 0xFF: // RST 38H
		cpu.Push(cpu.PC)
		cpu.PC = 0x0038
		cpu.MEMPTR = 0x0038
		return 11
	default:
		panic(fmt.Sprintf("Execute main unexpected code %x", opcode))
	}
}

// inc8 increments an 8-bit value and updates flags
func (cpu *CPU) inc8(value byte) byte {
	result := value + 1
	cpu.SetFlagState(FLAG_H, (value&0x0F) == 0x0F)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYFlags(result)
	// Set PV flag if incrementing 0x7F to 0x80 (overflow from positive to negative)
	cpu.SetFlagState(FLAG_PV, value == 0x7F)
	return result
}

// dec8 decrements an 8-bit value and updates flags
func (cpu *CPU) dec8(value byte) byte {
	result := value - 1
	cpu.SetFlagState(FLAG_H, (value&0x0F) == 0x00)
	cpu.SetFlag(FLAG_N, true)
	cpu.UpdateSZXYFlags(result)
	// Set PV flag if decrementing 0x80 to 0x7F (overflow from negative to positive)
	cpu.SetFlagState(FLAG_PV, value == 0x80)
	return result
}

// rlca rotates the accumulator left circular
func (cpu *CPU) rlca() {
	result := (cpu.A << 1) | (cpu.A >> 7)
	cpu.A = result
	cpu.SetFlagState(FLAG_C, (cpu.A&0x01) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateXYFlags(cpu.A)
}

// rla rotates the accumulator left through carry
func (cpu *CPU) rla() {
	oldCarry := cpu.GetFlag(FLAG_C)
	result := (cpu.A << 1)
	if oldCarry {
		result |= 0x01
	}
	cpu.SetFlagState(FLAG_C, (cpu.A&0x80) != 0)
	cpu.A = result
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateXYFlags(cpu.A)
}

// rrca rotates the accumulator right circular
func (cpu *CPU) rrca() {
	result := (cpu.A >> 1) | (cpu.A << 7)
	cpu.A = result
	cpu.SetFlagState(FLAG_C, (cpu.A&0x80) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateXYFlags(cpu.A)
}

// rra rotates the accumulator right through carry
func (cpu *CPU) rra() {
	oldCarry := cpu.GetFlag(FLAG_C)
	result := (cpu.A >> 1)
	if oldCarry {
		result |= 0x80
	}
	cpu.SetFlagState(FLAG_C, (cpu.A&0x01) != 0)
	cpu.A = result
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateXYFlags(cpu.A)
}

// daa performs decimal adjust on accumulator
func (cpu *CPU) daa() {
	temp := cpu.A
	correction := byte(0)
	carry := cpu.GetFlag(FLAG_C)

	if cpu.GetFlag(FLAG_H) || (cpu.A&0x0F) > 9 {
		correction |= 0x06
	}
	if carry || cpu.A > 0x99 {
		correction |= 0x60
	}

	if cpu.GetFlag(FLAG_N) {
		cpu.A -= correction
	} else {
		cpu.A += correction
	}

	cpu.SetFlag(FLAG_S, cpu.A&0x80 != 0)
	cpu.SetFlag(FLAG_Z, cpu.A == 0)
	cpu.SetFlag(FLAG_H, ((temp^correction^cpu.A)&0x10) != 0)
	cpu.SetFlag(FLAG_PV, cpu.parity(cpu.A))
	cpu.SetFlag(FLAG_C, carry || (correction&0x60 != 0))
	// Set X and Y flags from result
	cpu.SetFlag(FLAG_X, cpu.A&FLAG_X != 0)
	cpu.SetFlag(FLAG_Y, cpu.A&FLAG_Y != 0)
}

// Helper function to calculate parity
func (cpu *CPU) parity(val byte) bool {
	count := 0
	for i := 0; i < 8; i++ {
		if val&(1<<i) != 0 {
			count++
		}
	}
	return count%2 == 0
}

// cpl complements the accumulator
func (cpu *CPU) cpl() {
	cpu.A = ^cpu.A
	cpu.SetFlag(FLAG_H, true)
	cpu.SetFlag(FLAG_N, true)
	cpu.UpdateXYFlags(cpu.A)
}

// scf sets the carry flag
func (cpu *CPU) scf() {
	// // https://worldofspectrum.org/forums/discussion/41704
	cpu.SetFlag(FLAG_C, true)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	// This sets both flags if BOTH bits 3 and 5 are set in A, otherwise clears both
	if (cpu.A&FLAG_Y) == FLAG_Y && (cpu.A&FLAG_X) == FLAG_X {
		cpu.SetFlag(FLAG_Y, true)
		cpu.SetFlag(FLAG_X, true)
	}
	// // Note: If the condition is not met, the flags remain cleared (default behavior)
	// cpu.SetFlagState(FLAG_C, true)
	// cpu.SetFlagState(FLAG_N, false)
	// cpu.SetFlagState(FLAG_H, false)
	// // FIX: X and Y flags come from A register
	// cpu.SetFlagState(FLAG_X, cpu.A&FLAG_X != 0)
	// cpu.SetFlagState(FLAG_Y, cpu.A&FLAG_Y != 0)
}

// ccf complements the carry flag
func (cpu *CPU) ccf() {
	oldCarry := cpu.GetFlag(FLAG_C)
	cpu.SetFlagState(FLAG_C, !oldCarry)
	cpu.SetFlagState(FLAG_H, oldCarry) // H = old C
	cpu.ClearFlag(FLAG_N)
	// test fuse pass, test zexall failed ?
	if (cpu.A&FLAG_Y) == FLAG_Y && (cpu.A&FLAG_X) == FLAG_X {
		cpu.SetFlag(FLAG_Y, true)
		cpu.SetFlag(FLAG_X, true)
	}
	// FIX: X and Y flags come from A register
	//cpu.SetFlagState(FLAG_X, cpu.A&FLAG_X != 0)
	//cpu.SetFlagState(FLAG_Y, cpu.A&FLAG_Y != 0)
}

// add16 adds two 16-bit values and updates flags
func (cpu *CPU) add16(a, b uint16) uint16 {
	result := a + b
	cpu.SetFlagState(FLAG_C, result < a)
	cpu.SetFlagState(FLAG_H, (a&0x0FFF)+(b&0x0FFF) > 0x0FFF)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateFlags3and5FromAddress(result)
	return result
}

// add8 adds an 8-bit value to the accumulator and updates flags
func (cpu *CPU) add8(value byte) {
	a := cpu.A
	result := a + value
	cpu.SetFlagState(FLAG_C, result < a)
	cpu.SetFlagState(FLAG_H, (a&0x0F)+(value&0x0F) > 0x0F)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYFlags(result)
	// Set overflow flag: overflow occurs if adding two numbers with the same sign
	// produces a result with a different sign
	// Both operands have the same sign (both positive or both negative)
	// but the result has a different sign
	sameSign := (a^value)&0x80 == 0
	differentResultSign := (a^result)&0x80 != 0
	overflow := sameSign && differentResultSign
	cpu.SetFlagState(FLAG_PV, overflow)
	cpu.A = result
}

// adc8 adds an 8-bit value and carry to the accumulator and updates flags
func (cpu *CPU) adc8(value byte) {
	a := cpu.A
	carry := cpu.GetFlag(FLAG_C)
	var result byte
	if carry {
		result = a + value + 1
	} else {
		result = a + value
	}
	cpu.SetFlagState(FLAG_C, int(a)+int(value)+int(boolToByte(carry)) > 0xFF)
	cpu.SetFlagState(FLAG_H, (a&0x0F)+(value&0x0F)+boolToByte(carry) > 0x0F)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYFlags(result)
	// Set overflow flag: overflow occurs if adding two numbers with the same sign
	// produces a result with a different sign
	originalValue := a
	if carry {
		sameSign := (originalValue^value)&0x80 == 0
		differentResultSign := (originalValue^result)&0x80 != 0
		overflow := sameSign && differentResultSign
		cpu.SetFlagState(FLAG_PV, overflow)
	} else {
		sameSign := (originalValue^value)&0x80 == 0
		differentResultSign := (originalValue^result)&0x80 != 0
		overflow := sameSign && differentResultSign
		cpu.SetFlagState(FLAG_PV, overflow)
	}
	cpu.A = result
}

// sub8 subtracts an 8-bit value from the accumulator and updates flags
func (cpu *CPU) sub8(value byte) {
	a := cpu.A
	result := a - value
	cpu.SetFlagState(FLAG_C, a < value)
	cpu.SetFlagState(FLAG_H, (a&0x0F) < (value&0x0F))
	cpu.SetFlag(FLAG_N, true)
	cpu.UpdateSZXYFlags(result)
	// Set overflow flag: overflow occurs if subtracting two numbers with different signs
	// produces a result with the same sign as the subtrahend
	overflow := ((a^value)&0x80) != 0 && ((a^result)&0x80) != 0
	cpu.SetFlagState(FLAG_PV, overflow)
	cpu.A = result
}

// sbc8 subtracts an 8-bit value and carry from the accumulator and updates flags
func (cpu *CPU) sbc8(value byte) {
	a := cpu.A
	carry := cpu.GetFlag(FLAG_C)
	var result byte
	if carry {
		result = a - value - 1
	} else {
		result = a - value
	}
	cpu.SetFlagState(FLAG_C, int(a)-int(value)-int(boolToByte(carry)) < 0)
	cpu.SetFlagState(FLAG_H, (a&0x0F) < (value&0x0F)+boolToByte(carry))
	cpu.SetFlag(FLAG_N, true)
	cpu.UpdateSZXYFlags(result)
	// Set overflow flag: overflow occurs if subtracting two numbers with different signs
	// produces a result with the same sign as the subtrahend
	originalValue := a
	if carry {
		overflow := ((originalValue^value)&0x80) != 0 && ((originalValue^result)&0x80) != 0
		cpu.SetFlagState(FLAG_PV, overflow)
	} else {
		overflow := ((originalValue^value)&0x80) != 0 && ((originalValue^result)&0x80) != 0
		cpu.SetFlagState(FLAG_PV, overflow)
	}
	cpu.A = result
}

// and8 performs bitwise AND with the accumulator and updates flags
func (cpu *CPU) and8(value byte) {
	cpu.A &= value
	cpu.ClearFlag(FLAG_C)
	cpu.ClearFlag(FLAG_N)
	cpu.SetFlag(FLAG_H, true)
	cpu.UpdateSZXYFlags(cpu.A)
	// For logical operations, P/V flag indicates parity
	cpu.SetFlagState(FLAG_PV, cpu.parity(cpu.A))
}

// xor8 performs bitwise XOR with the accumulator and updates flags
func (cpu *CPU) xor8(value byte) {
	cpu.A ^= value
	cpu.ClearFlag(FLAG_C)
	cpu.ClearFlag(FLAG_N)
	cpu.ClearFlag(FLAG_H)
	cpu.UpdateSZXYFlags(cpu.A)
	// For logical operations, P/V flag indicates parity
	cpu.SetFlagState(FLAG_PV, cpu.parity(cpu.A))
}

// or8 performs bitwise OR with the accumulator and updates flags
func (cpu *CPU) or8(value byte) {
	cpu.A |= value
	cpu.ClearFlag(FLAG_C)
	cpu.ClearFlag(FLAG_N)
	cpu.ClearFlag(FLAG_H)
	cpu.UpdateSZXYFlags(cpu.A)
	// For logical operations, P/V flag indicates parity
	cpu.SetFlagState(FLAG_PV, cpu.parity(cpu.A))
}

// cp8 compares an 8-bit value with the accumulator and updates flags
func (cpu *CPU) cp8(value byte) {
	a := cpu.A
	result := a - value
	cpu.SetFlagState(FLAG_C, a < value)
	cpu.SetFlagState(FLAG_H, (a&0x0F) < (value&0x0F))
	cpu.SetFlag(FLAG_N, true)
	cpu.UpdateSZFlags(result)
	// For CP instruction, X and Y flags are set from the operand, not the result
	cpu.UpdateFlags3and5FromValue(value)
	// Set overflow flag: overflow occurs if subtracting two numbers with different signs
	// produces a result with the same sign as the subtrahend
	overflow := ((a^value)&0x80) != 0 && ((a^result)&0x80) != 0
	cpu.SetFlagState(FLAG_PV, overflow)
}
