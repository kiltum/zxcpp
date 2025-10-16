// Package z80 implements a Z80 CPU emulator with support for all documented
// and undocumented opcodes, flags, and registers.
package z80

// ExecuteDDOpcode executes a DD-prefixed opcode and returns the number of T-states used
func (cpu *CPU) ExecuteDDOpcode(opcode byte) int {
	switch opcode {
	// Load instructions
	case 0x09: // ADD IX, BC
		oldIX := cpu.IX
		result := cpu.add16IX(cpu.IX, cpu.GetBC())
		cpu.MEMPTR = oldIX + 1
		cpu.IX = result
		return 15
	case 0x19: // ADD IX, DE
		oldIX := cpu.IX
		result := cpu.add16IX(cpu.IX, cpu.GetDE())
		cpu.MEMPTR = oldIX + 1
		cpu.IX = result
		return 15
	case 0x21: // LD IX, nn
		cpu.IX = cpu.ReadImmediateWord()
		return 14
	case 0x22: // LD (nn), IX
		addr := cpu.ReadImmediateWord()
		cpu.Memory.WriteWord(addr, cpu.IX)
		cpu.MEMPTR = addr + 1
		return 20
	case 0x23: // INC IX
		cpu.IX++
		return 10
	case 0x24: // INC IXH
		cpu.SetIXH(cpu.inc8(cpu.GetIXH()))
		return 8
	case 0x25: // DEC IXH
		cpu.SetIXH(cpu.dec8(cpu.GetIXH()))
		return 8
	case 0x26: // LD IXH, n
		cpu.SetIXH(cpu.ReadImmediateByte())
		return 11
	case 0x29: // ADD IX, IX
		oldIX := cpu.IX
		result := cpu.add16IX(cpu.IX, cpu.IX)
		cpu.MEMPTR = oldIX + 1
		cpu.IX = result
		return 15
	case 0x2A: // LD IX, (nn)
		addr := cpu.ReadImmediateWord()
		cpu.IX = cpu.Memory.ReadWord(addr)
		cpu.MEMPTR = addr + 1
		return 20
	case 0x2B: // DEC IX
		cpu.IX--
		return 10
	case 0x2C: // INC IXL
		cpu.SetIXL(cpu.inc8(cpu.GetIXL()))
		return 8
	case 0x2D: // DEC IXL
		cpu.SetIXL(cpu.dec8(cpu.GetIXL()))
		return 8
	case 0x2E: // LD IXL, n
		cpu.SetIXL(cpu.ReadImmediateByte())
		return 11
	case 0x34: // INC (IX+d)
		return cpu.executeIncDecIndexed(true)
	case 0x35: // DEC (IX+d)
		return cpu.executeIncDecIndexed(false)
	case 0x36: // LD (IX+d), n
		displacement := cpu.ReadDisplacement()
		value := cpu.ReadImmediateByte()
		addr := uint16(int32(cpu.IX) + int32(displacement))
		cpu.Memory.WriteByte(addr, value)
		cpu.MEMPTR = addr
		return 19
	case 0x39: // ADD IX, SP
		oldIX := cpu.IX
		result := cpu.add16IX(cpu.IX, cpu.SP)
		cpu.MEMPTR = oldIX + 1
		cpu.IX = result
		return 15
	case 0x40: // LD B,B
		return 8

	// Load register from IX register
	case 0x44: // LD B, IXH
		cpu.B = cpu.GetIXH()
		return 8
	case 0x45: // LD B, IXL
		cpu.B = cpu.GetIXL()
		return 8
	case 0x46: // LD B, (IX+d)
		return cpu.executeLoadFromIndexed(0)
	case 0x4C: // LD C, IXH
		cpu.C = cpu.GetIXH()
		return 8
	case 0x4D: // LD C, IXL
		cpu.C = cpu.GetIXL()
		return 8
	case 0x4E: // LD C, (IX+d)
		return cpu.executeLoadFromIndexed(1)
	case 0x54: // LD D, IXH
		cpu.D = cpu.GetIXH()
		return 8
	case 0x55: // LD D, IXL
		cpu.D = cpu.GetIXL()
		return 8
	case 0x56: // LD D, (IX+d)
		return cpu.executeLoadFromIndexed(2)
	case 0x5C: // LD E, IXH
		cpu.E = cpu.GetIXH()
		return 8
	case 0x5D: // LD E, IXL
		cpu.E = cpu.GetIXL()
		return 8
	case 0x5E: // LD E, (IX+d)
		return cpu.executeLoadFromIndexed(3)
	case 0x60: // LD IXH, B
		cpu.SetIXH(cpu.B)
		return 8
	case 0x61: // LD IXH, C
		cpu.SetIXH(cpu.C)
		return 8
	case 0x62: // LD IXH, D
		cpu.SetIXH(cpu.D)
		return 8
	case 0x63: // LD IXH, E
		cpu.SetIXH(cpu.E)
		return 8
	case 0x64: // LD IXH, IXH
		// No operation needed
		return 8
	case 0x65: // LD IXH, IXL
		cpu.SetIXH(cpu.GetIXL())
		return 8
	case 0x66: // LD H, (IX+d)
		return cpu.executeLoadFromIndexed(4)
	case 0x67: // LD IXH, A
		cpu.SetIXH(cpu.A)
		return 8
	case 0x68: // LD IXL, B
		cpu.SetIXL(cpu.B)
		return 8
	case 0x69: // LD IXL, C
		cpu.SetIXL(cpu.C)
		return 8
	case 0x6A: // LD IXL, D
		cpu.SetIXL(cpu.D)
		return 8
	case 0x6B: // LD IXL, E
		cpu.SetIXL(cpu.E)
		return 8
	case 0x6C: // LD IXL, IXH
		cpu.SetIXL(cpu.GetIXH())
		return 8
	case 0x6D: // LD IXL, IXL
		// No operation needed
		return 8
	case 0x6E: // LD L, (IX+d)
		return cpu.executeLoadFromIndexed(5)
	case 0x6F: // LD IXL, A
		cpu.SetIXL(cpu.A)
		return 8
	case 0x70: // LD (IX+d), B
		return cpu.executeStoreToIndexed(cpu.B)
	case 0x71: // LD (IX+d), C
		return cpu.executeStoreToIndexed(cpu.C)
	case 0x72: // LD (IX+d), D
		return cpu.executeStoreToIndexed(cpu.D)
	case 0x73: // LD (IX+d), E
		return cpu.executeStoreToIndexed(cpu.E)
	case 0x74: // LD (IX+d), H
		return cpu.executeStoreToIndexed(cpu.H)
	case 0x75: // LD (IX+d), L
		return cpu.executeStoreToIndexed(cpu.L)
	case 0x77: // LD (IX+d), A
		return cpu.executeStoreToIndexed(cpu.A)
	case 0x7C: // LD A, IXH
		cpu.A = cpu.GetIXH()
		return 8
	case 0x7D: // LD A, IXL
		cpu.A = cpu.GetIXL()
		return 8
	case 0x7E: // LD A, (IX+d)
		return cpu.executeLoadFromIndexed(7)

	// Arithmetic and logic instructions
	case 0x84: // ADD A, IXH
		cpu.add8(cpu.GetIXH())
		return 8
	case 0x85: // ADD A, IXL
		cpu.add8(cpu.GetIXL())
		return 8
	case 0x86: // ADD A, (IX+d)
		return cpu.executeALUIndexed(0)
	case 0x8C: // ADC A, IXH
		cpu.adc8(cpu.GetIXH())
		return 8
	case 0x8D: // ADC A, IXL
		cpu.adc8(cpu.GetIXL())
		return 8
	case 0x8E: // ADC A, (IX+d)
		return cpu.executeALUIndexed(1)
	case 0x94: // SUB IXH
		cpu.sub8(cpu.GetIXH())
		return 8
	case 0x95: // SUB IXL
		cpu.sub8(cpu.GetIXL())
		return 8
	case 0x96: // SUB (IX+d)
		return cpu.executeALUIndexed(2)
	case 0x9C: // SBC A, IXH
		cpu.sbc8(cpu.GetIXH())
		return 8
	case 0x9D: // SBC A, IXL
		cpu.sbc8(cpu.GetIXL())
		return 8
	case 0x9E: // SBC A, (IX+d)
		return cpu.executeALUIndexed(3)
	case 0xA4: // AND IXH
		cpu.and8(cpu.GetIXH())
		return 8
	case 0xA5: // AND IXL
		cpu.and8(cpu.GetIXL())
		return 8
	case 0xA6: // AND (IX+d)
		return cpu.executeALUIndexed(4)
	case 0xAC: // XOR IXH
		cpu.xor8(cpu.GetIXH())
		return 8
	case 0xAD: // XOR IXL
		cpu.xor8(cpu.GetIXL())
		return 8
	case 0xAE: // XOR (IX+d)
		return cpu.executeALUIndexed(5)
	case 0xB4: // OR IXH
		cpu.or8(cpu.GetIXH())
		return 8
	case 0xB5: // OR IXL
		cpu.or8(cpu.GetIXL())
		return 8
	case 0xB6: // OR (IX+d)
		return cpu.executeALUIndexed(6)
	case 0xBC: // CP IXH
		cpu.cp8(cpu.GetIXH())
		return 8
	case 0xBD: // CP IXL
		cpu.cp8(cpu.GetIXL())
		return 8
	case 0xBE: // CP (IX+d)
		return cpu.executeALUIndexed(7)

	// POP and PUSH instructions
	case 0xE1: // POP IX
		cpu.IX = cpu.Pop()
		return 14
	case 0xE3: // EX (SP), IX
		temp := cpu.Memory.ReadWord(cpu.SP)
		cpu.Memory.WriteWord(cpu.SP, cpu.IX)
		cpu.IX = temp
		cpu.MEMPTR = temp
		return 23
	case 0xE5: // PUSH IX
		cpu.Push(cpu.IX)
		return 15
	case 0xE9: // JP (IX)
		cpu.PC = cpu.IX
		return 8
	case 0xF9: // LD SP, IX
		cpu.SP = cpu.IX
		return 10

	// Handle DD CB prefix (IX with displacement and CB operations)
	case 0xCB: // DD CB prefix
		return cpu.executeDDCBOpcode()

	case 0xfd:
		return 8
	case 0x00: // Extended NOP (undocumented)
		// DD 00 is an undocumented instruction that acts as an extended NOP
		// It consumes the DD prefix and the 00 opcode but executes as a NOP
		// Takes 8 cycles total (4 for DD prefix fetch + 4 for 00 opcode fetch)
		return 8
	case 0xdd:
		return 8
	default:
		return cpu.ExecuteOpcode(opcode)
		//panic(fmt.Sprintf("DD unexpected code %x", opcode))
	}
}

// executeIncDecIndexed handles INC/DEC (IX+d) instructions
func (cpu *CPU) executeIncDecIndexed(isInc bool) int {
	displacement := cpu.ReadDisplacement()
	addr := uint16(int32(cpu.IX) + int32(displacement))
	value := cpu.Memory.ReadByte(addr)
	var result byte
	if isInc {
		result = cpu.inc8(value)
	} else {
		result = cpu.dec8(value)
	}
	cpu.Memory.WriteByte(addr, result)
	cpu.MEMPTR = addr
	return 23
}

// executeLoadFromIndexed handles LD r, (IX+d) instructions
func (cpu *CPU) executeLoadFromIndexed(reg byte) int {
	displacement := cpu.ReadDisplacement()
	addr := uint16(int32(cpu.IX) + int32(displacement))
	value := cpu.Memory.ReadByte(addr)

	switch reg {
	case 0:
		cpu.B = value
	case 1:
		cpu.C = value
	case 2:
		cpu.D = value
	case 3:
		cpu.E = value
	case 4:
		cpu.H = value
	case 5:
		cpu.L = value
	case 7:
		cpu.A = value
	}

	cpu.MEMPTR = addr
	return 19
}

// executeStoreToIndexed handles LD (IX+d), r instructions
func (cpu *CPU) executeStoreToIndexed(value byte) int {
	displacement := cpu.ReadDisplacement()
	addr := uint16(int32(cpu.IX) + int32(displacement))
	cpu.Memory.WriteByte(addr, value)
	cpu.MEMPTR = addr
	return 19
}

// executeALUIndexed handles ALU operations with (IX+d) operand
func (cpu *CPU) executeALUIndexed(opType byte) int {
	displacement := cpu.ReadDisplacement()
	addr := uint16(int32(cpu.IX) + int32(displacement))
	value := cpu.Memory.ReadByte(addr)

	switch opType {
	case 0: // ADD
		cpu.add8(value)
	case 1: // ADC
		cpu.adc8(value)
	case 2: // SUB
		cpu.sub8(value)
	case 3: // SBC
		cpu.sbc8(value)
	case 4: // AND
		cpu.and8(value)
	case 5: // XOR
		cpu.xor8(value)
	case 6: // OR
		cpu.or8(value)
	case 7: // CP
		cpu.cp8(value)
	}

	cpu.MEMPTR = addr
	return 19
}

// add16IX adds two 16-bit values for IX register and updates flags
func (cpu *CPU) add16IX(a, b uint16) uint16 {
	result := a + b
	cpu.SetFlagState(FLAG_C, result < a)
	cpu.SetFlagState(FLAG_H, (a&0x0FFF)+(b&0x0FFF) > 0x0FFF)
	cpu.ClearFlag(FLAG_N)
	// For IX operations, we update X and Y flags from high byte of result
	cpu.UpdateFlags3and5FromAddress(result)
	return result
}
