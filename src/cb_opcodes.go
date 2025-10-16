// Package z80 implements a Z80 CPU emulator with support for all documented
// and undocumented opcodes, flags, and registers.
package z80

// ExecuteCBOpcode executes a CB-prefixed opcode and returns the number of T-states used
func (cpu *CPU) ExecuteCBOpcode(opcode byte) int {
	// Handle rotate and shift instructions (0x00-0x3F)
	if opcode <= 0x3F {
		// Determine operation type from opcode bits 3-5
		opType := (opcode >> 3) & 0x07
		// Determine register from opcode bits 0-2
		reg := opcode & 0x07

		// Handle (HL) special case
		if reg == 6 {
			addr := cpu.GetHL()
			value := cpu.Memory.ReadByte(addr)

			switch opType {
			case 0: // RLC
				result := cpu.rlc(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 1: // RRC
				result := cpu.rrc(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 2: // RL
				result := cpu.rl(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 3: // RR
				result := cpu.rr(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 4: // SLA
				result := cpu.sla(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 5: // SRA
				result := cpu.sra(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 6: // SLL (Undocumented)
				result := cpu.sll(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			case 7: // SRL
				result := cpu.srl(value)
				cpu.Memory.WriteByte(addr, result)
				return 15
			}
		} else {
			// Handle regular registers
			switch opType {
			case 0: // RLC
				switch reg {
				case 0:
					cpu.B = cpu.rlc(cpu.B)
				case 1:
					cpu.C = cpu.rlc(cpu.C)
				case 2:
					cpu.D = cpu.rlc(cpu.D)
				case 3:
					cpu.E = cpu.rlc(cpu.E)
				case 4:
					cpu.H = cpu.rlc(cpu.H)
				case 5:
					cpu.L = cpu.rlc(cpu.L)
				case 7:
					cpu.A = cpu.rlc(cpu.A)
				}
				return 8
			case 1: // RRC
				switch reg {
				case 0:
					cpu.B = cpu.rrc(cpu.B)
				case 1:
					cpu.C = cpu.rrc(cpu.C)
				case 2:
					cpu.D = cpu.rrc(cpu.D)
				case 3:
					cpu.E = cpu.rrc(cpu.E)
				case 4:
					cpu.H = cpu.rrc(cpu.H)
				case 5:
					cpu.L = cpu.rrc(cpu.L)
				case 7:
					cpu.A = cpu.rrc(cpu.A)
				}
				return 8
			case 2: // RL
				switch reg {
				case 0:
					cpu.B = cpu.rl(cpu.B)
				case 1:
					cpu.C = cpu.rl(cpu.C)
				case 2:
					cpu.D = cpu.rl(cpu.D)
				case 3:
					cpu.E = cpu.rl(cpu.E)
				case 4:
					cpu.H = cpu.rl(cpu.H)
				case 5:
					cpu.L = cpu.rl(cpu.L)
				case 7:
					cpu.A = cpu.rl(cpu.A)
				}
				return 8
			case 3: // RR
				switch reg {
				case 0:
					cpu.B = cpu.rr(cpu.B)
				case 1:
					cpu.C = cpu.rr(cpu.C)
				case 2:
					cpu.D = cpu.rr(cpu.D)
				case 3:
					cpu.E = cpu.rr(cpu.E)
				case 4:
					cpu.H = cpu.rr(cpu.H)
				case 5:
					cpu.L = cpu.rr(cpu.L)
				case 7:
					cpu.A = cpu.rr(cpu.A)
				}
				return 8
			case 4: // SLA
				switch reg {
				case 0:
					cpu.B = cpu.sla(cpu.B)
				case 1:
					cpu.C = cpu.sla(cpu.C)
				case 2:
					cpu.D = cpu.sla(cpu.D)
				case 3:
					cpu.E = cpu.sla(cpu.E)
				case 4:
					cpu.H = cpu.sla(cpu.H)
				case 5:
					cpu.L = cpu.sla(cpu.L)
				case 7:
					cpu.A = cpu.sla(cpu.A)
				}
				return 8
			case 5: // SRA
				switch reg {
				case 0:
					cpu.B = cpu.sra(cpu.B)
				case 1:
					cpu.C = cpu.sra(cpu.C)
				case 2:
					cpu.D = cpu.sra(cpu.D)
				case 3:
					cpu.E = cpu.sra(cpu.E)
				case 4:
					cpu.H = cpu.sra(cpu.H)
				case 5:
					cpu.L = cpu.sra(cpu.L)
				case 7:
					cpu.A = cpu.sra(cpu.A)
				}
				return 8
			case 6: // SLL (Undocumented)
				switch reg {
				case 0:
					cpu.B = cpu.sll(cpu.B)
				case 1:
					cpu.C = cpu.sll(cpu.C)
				case 2:
					cpu.D = cpu.sll(cpu.D)
				case 3:
					cpu.E = cpu.sll(cpu.E)
				case 4:
					cpu.H = cpu.sll(cpu.H)
				case 5:
					cpu.L = cpu.sll(cpu.L)
				case 7:
					cpu.A = cpu.sll(cpu.A)
				}
				return 8
			case 7: // SRL
				switch reg {
				case 0:
					cpu.B = cpu.srl(cpu.B)
				case 1:
					cpu.C = cpu.srl(cpu.C)
				case 2:
					cpu.D = cpu.srl(cpu.D)
				case 3:
					cpu.E = cpu.srl(cpu.E)
				case 4:
					cpu.H = cpu.srl(cpu.H)
				case 5:
					cpu.L = cpu.srl(cpu.L)
				case 7:
					cpu.A = cpu.srl(cpu.A)
				}
				return 8
			}
		}
	}

	// Handle bit test instructions (0x40-0x7F)
	if opcode >= 0x40 && opcode <= 0x7F {
		bitNum := uint((opcode >> 3) & 0x07)
		reg := opcode & 0x07

		// Handle (HL) special case
		if reg == 6 {
			value := cpu.Memory.ReadByte(cpu.GetHL())
			cpu.bitMem(bitNum, value, byte(cpu.MEMPTR>>8))
			return 12
		} else {
			// Handle regular registers
			var regValue byte
			switch reg {
			case 0:
				regValue = cpu.B
			case 1:
				regValue = cpu.C
			case 2:
				regValue = cpu.D
			case 3:
				regValue = cpu.E
			case 4:
				regValue = cpu.H
			case 5:
				regValue = cpu.L
			case 7:
				regValue = cpu.A
			}
			cpu.bit(bitNum, regValue)
			return 8
		}
	}

	// Handle reset bit instructions (0x80-0xBF)
	if opcode >= 0x80 && opcode <= 0xBF {
		bitNum := uint((opcode >> 3) & 0x07)
		reg := opcode & 0x07

		// Handle (HL) special case
		if reg == 6 {
			addr := cpu.GetHL()
			value := cpu.Memory.ReadByte(addr)
			result := cpu.res(bitNum, value)
			cpu.Memory.WriteByte(addr, result)
			return 15
		} else {
			// Handle regular registers
			switch reg {
			case 0:
				cpu.B = cpu.res(bitNum, cpu.B)
			case 1:
				cpu.C = cpu.res(bitNum, cpu.C)
			case 2:
				cpu.D = cpu.res(bitNum, cpu.D)
			case 3:
				cpu.E = cpu.res(bitNum, cpu.E)
			case 4:
				cpu.H = cpu.res(bitNum, cpu.H)
			case 5:
				cpu.L = cpu.res(bitNum, cpu.L)
			case 7:
				cpu.A = cpu.res(bitNum, cpu.A)
			}
			return 8
		}
	}

	// Handle set bit instructions (0xC0-0xFF)
	if opcode >= 0xC0 {
		bitNum := uint((opcode >> 3) & 0x07)
		reg := opcode & 0x07

		// Handle (HL) special case
		if reg == 6 {
			addr := cpu.GetHL()
			value := cpu.Memory.ReadByte(addr)
			result := cpu.set(bitNum, value)
			cpu.Memory.WriteByte(addr, result)
			return 15
		} else {
			// Handle regular registers
			switch reg {
			case 0:
				cpu.B = cpu.set(bitNum, cpu.B)
			case 1:
				cpu.C = cpu.set(bitNum, cpu.C)
			case 2:
				cpu.D = cpu.set(bitNum, cpu.D)
			case 3:
				cpu.E = cpu.set(bitNum, cpu.E)
			case 4:
				cpu.H = cpu.set(bitNum, cpu.H)
			case 5:
				cpu.L = cpu.set(bitNum, cpu.L)
			case 7:
				cpu.A = cpu.set(bitNum, cpu.A)
			}
			return 8
		}
	}

	// Unimplemented opcode
	return 8
}

// rlc rotates a byte left circular
func (cpu *CPU) rlc(value byte) byte {
	result := (value << 1) | (value >> 7)
	cpu.SetFlagState(FLAG_C, (value&0x80) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// rrc rotates a byte right circular
func (cpu *CPU) rrc(value byte) byte {
	result := (value >> 1) | (value << 7)
	cpu.SetFlagState(FLAG_C, (value&0x01) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// rl rotates a byte left through carry
func (cpu *CPU) rl(value byte) byte {
	oldCarry := cpu.GetFlag(FLAG_C)
	result := (value << 1)
	if oldCarry {
		result |= 0x01
	}
	cpu.SetFlagState(FLAG_C, (value&0x80) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// rr rotates a byte right through carry
func (cpu *CPU) rr(value byte) byte {
	oldCarry := cpu.GetFlag(FLAG_C)
	result := (value >> 1)
	if oldCarry {
		result |= 0x80
	}
	cpu.SetFlagState(FLAG_C, (value&0x01) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// sla shifts a byte left arithmetic
func (cpu *CPU) sla(value byte) byte {
	result := value << 1
	cpu.SetFlagState(FLAG_C, (value&0x80) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// sra shifts a byte right arithmetic
func (cpu *CPU) sra(value byte) byte {
	result := (value >> 1) | (value & 0x80)
	cpu.SetFlagState(FLAG_C, (value&0x01) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// sll shifts a byte left logical (Undocumented)
func (cpu *CPU) sll(value byte) byte {
	result := (value << 1) | 0x01
	cpu.SetFlagState(FLAG_C, (value&0x80) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// srl shifts a byte right logical
func (cpu *CPU) srl(value byte) byte {
	result := value >> 1
	cpu.SetFlagState(FLAG_C, (value&0x01) != 0)
	cpu.ClearFlag(FLAG_H)
	cpu.ClearFlag(FLAG_N)
	cpu.UpdateSZXYPVFlags(result)
	return result
}

// bit tests a bit in a byte
func (cpu *CPU) bit(bitNum uint, value byte) {
	mask := byte(1 << bitNum)
	result := value & mask
	cpu.SetFlagState(FLAG_Z, result == 0)
	cpu.SetFlagState(FLAG_Y, (value&(1<<5)) != 0)
	cpu.SetFlagState(FLAG_X, (value&(1<<3)) != 0)
	cpu.SetFlag(FLAG_H, true)
	cpu.ClearFlag(FLAG_N)
	if result == 0 {
		cpu.SetFlag(FLAG_PV, true)
		cpu.ClearFlag(FLAG_S)
	} else {
		cpu.ClearFlag(FLAG_PV)
		// For BIT 7, S flag is set to the value of bit 7
		if bitNum == 7 {
			cpu.SetFlagState(FLAG_S, (value&0x80) != 0)
		} else {
			cpu.ClearFlag(FLAG_S)
		}
	}
}

// res resets a bit in a byte
func (cpu *CPU) res(bitNum uint, value byte) byte {
	mask := byte(^(1 << bitNum))
	return value & mask
}

// bitMem tests a bit in a byte for memory references
func (cpu *CPU) bitMem(bitNum uint, value byte, addrHi byte) {
	mask := byte(1 << bitNum)
	result := value & mask
	cpu.SetFlagState(FLAG_Z, result == 0)
	cpu.SetFlagState(FLAG_Y, (addrHi&(1<<5)) != 0)
	cpu.SetFlagState(FLAG_X, (addrHi&(1<<3)) != 0)
	cpu.SetFlag(FLAG_H, true)
	cpu.ClearFlag(FLAG_N)
	if result == 0 {
		cpu.SetFlag(FLAG_PV, true)
		cpu.ClearFlag(FLAG_S)
	} else {
		cpu.ClearFlag(FLAG_PV)
		// For BIT 7, S flag is set to the value of bit 7
		if bitNum == 7 {
			cpu.SetFlagState(FLAG_S, (value&0x80) != 0)
		} else {
			cpu.ClearFlag(FLAG_S)
		}
	}
}

// set sets a bit in a byte
func (cpu *CPU) set(bitNum uint, value byte) byte {
	mask := byte(1 << bitNum)
	return value | mask
}
