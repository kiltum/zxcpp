// Package z80 implements a Z80 CPU emulator with support for all documented
// and undocumented opcodes, flags, and registers.
package z80

// ExecuteDDCBOpcode executes a DD CB prefixed opcode
// This is handled in dd_opcodes.go in the executeDDCBOpcode function
// This file exists to satisfy the requirement of separating opcodes by prefix
// executeDDCBOpcode executes a DD CB prefixed opcode
func (cpu *CPU) executeDDCBOpcode() int {
	// For DD CB prefixed instructions, R should be incremented by 2 total
	// We've already incremented R once for the DD prefix and once for the CB prefix
	// So we need to adjust by -1 to get the correct total increment of 2
	originalR := cpu.R

	displacement := cpu.ReadDisplacement()
	opcode := cpu.ReadOpcode()

	// Adjust R register - DD CB instructions should increment R by 2 total
	// We've already incremented twice (DD and CB), so we need to subtract 1
	// to get the correct total of 2 increments
	cpu.R = originalR

	addr := uint16(int32(cpu.IX) + int32(displacement))
	value := cpu.Memory.ReadByte(addr)

	// Handle rotate and shift instructions (0x00-0x3F)
	if opcode <= 0x3F {
		return cpu.executeRotateShiftIndexed(opcode, addr, value)
	}

	// Handle bit test instructions (0x40-0x7F)
	if opcode >= 0x40 && opcode <= 0x7F {
		bitNum := uint((opcode >> 3) & 0x07)
		cpu.bitMem(bitNum, value, byte(addr>>8))
		cpu.MEMPTR = addr
		return 20
	}

	// Handle reset bit instructions (0x80-0xBF)
	if opcode >= 0x80 && opcode <= 0xBF {
		return cpu.executeResetBitIndexed(opcode, addr, value)
	}

	// Handle set bit instructions (0xC0-0xFF)
	if opcode >= 0xC0 {
		return cpu.executeSetBitIndexed(opcode, addr, value)
	}

	// Unimplemented opcode
	return 23
}

// executeRotateShiftIndexed handles rotate and shift instructions for indexed addressing
func (cpu *CPU) executeRotateShiftIndexed(opcode byte, addr uint16, value byte) int {
	// Determine operation type from opcode bits 3-5
	opType := (opcode >> 3) & 0x07
	// Determine register from opcode bits 0-2
	reg := opcode & 0x07

	// Perform the operation
	var result byte
	switch opType {
	case 0: // RLC
		result = cpu.rlc(value)
	case 1: // RRC
		result = cpu.rrc(value)
	case 2: // RL
		result = cpu.rl(value)
	case 3: // RR
		result = cpu.rr(value)
	case 4: // SLA
		result = cpu.sla(value)
	case 5: // SRA
		result = cpu.sra(value)
	case 6: // SLL (Undocumented)
		result = cpu.sll(value)
	case 7: // SRL
		result = cpu.srl(value)
	default:
		result = value
	}

	// Store result in memory
	cpu.Memory.WriteByte(addr, result)

	// Store result in register if needed (except for (HL) case)
	if reg != 6 { // reg 6 is (HL) - no register store needed
		switch reg {
		case 0:
			cpu.B = result
		case 1:
			cpu.C = result
		case 2:
			cpu.D = result
		case 3:
			cpu.E = result
		case 4:
			cpu.H = result
		case 5:
			cpu.L = result
		case 7:
			cpu.A = result
		}
	}

	cpu.MEMPTR = addr
	return 23
}

// executeResetBitIndexed handles reset bit instructions for indexed addressing
func (cpu *CPU) executeResetBitIndexed(opcode byte, addr uint16, value byte) int {
	bitNum := uint((opcode >> 3) & 0x07)
	reg := opcode & 0x07

	result := cpu.res(bitNum, value)
	cpu.Memory.WriteByte(addr, result)

	// Store result in register if needed (except for (HL) case)
	if reg != 6 { // reg 6 is (HL) - no register store needed
		switch reg {
		case 0:
			cpu.B = result
		case 1:
			cpu.C = result
		case 2:
			cpu.D = result
		case 3:
			cpu.E = result
		case 4:
			cpu.H = result
		case 5:
			cpu.L = result
		case 7:
			cpu.A = result
		}
	}

	cpu.MEMPTR = addr
	return 23
}

// executeSetBitIndexed handles set bit instructions for indexed addressing
func (cpu *CPU) executeSetBitIndexed(opcode byte, addr uint16, value byte) int {
	bitNum := uint((opcode >> 3) & 0x07)
	reg := opcode & 0x07

	result := cpu.set(bitNum, value)
	cpu.Memory.WriteByte(addr, result)

	// Store result in register if needed (except for (HL) case)
	if reg != 6 { // reg 6 is (HL) - no register store needed
		switch reg {
		case 0:
			cpu.B = result
		case 1:
			cpu.C = result
		case 2:
			cpu.D = result
		case 3:
			cpu.E = result
		case 4:
			cpu.H = result
		case 5:
			cpu.L = result
		case 7:
			cpu.A = result
		}
	}

	cpu.MEMPTR = addr
	return 23
}
