// Package z80 implements a Z80 CPU emulator with support for all documented
// and undocumented opcodes, flags, and registers.
package z80


// Memory interface for memory operations
type Memory interface {
	ReadByte(address uint16) byte
	WriteByte(address uint16, value byte)
	ReadWord(address uint16) uint16
	WriteWord(address uint16, value uint16)
}

// IO interface for input/output operations
type IO interface {
	ReadPort(port uint16) byte
	WritePort(port uint16, value byte)
	CheckInterrupt() bool
}

// FLAG_* constants represent the bit positions of the FLAGS register
const (
	FLAG_C  = 0x01 // Carry flag
	FLAG_N  = 0x02 // Add/Subtract flag
	FLAG_PV = 0x04 // Parity/Overflow flag
	FLAG_3  = 0x08 // Undocumented 3rd bit flag
	FLAG_H  = 0x10 // Half Carry flag
	FLAG_5  = 0x20 // Undocumented 5th bit flag
	FLAG_Z  = 0x40 // Zero flag
	FLAG_S  = 0x80 // Sign flag
)

// FLAG_X and FLAG_Y are aliases for FLAG_3 and FLAG_5 respectively
const (
	FLAG_X = FLAG_3
	FLAG_Y = FLAG_5
)

// CPU represents the state of a Z80 processor
type CPU struct {
	A      byte   // Accumulator
	F      byte   // Flags register
	B, C   byte   // BC register pair
	D, E   byte   // DE register pair
	H, L   byte   // HL register pair
	A_, F_ byte   // Alternate AF register pair
	B_, C_ byte   // Alternate BC register pair
	D_, E_ byte   // Alternate DE register pair
	H_, L_ byte   // Alternate HL register pair
	IX     uint16 // Index register X
	IY     uint16 // Index register Y
	SP     uint16 // Stack pointer
	PC     uint16 // Program counter
	I      byte   // Interrupt vector
	R      byte   // Memory refresh
	IM     byte   // Interrupt mode (0, 1, or 2)
	IFF1   bool   // Interrupt flip-flop 1
	IFF2   bool   // Interrupt flip-flop 2
	HALT   bool   // HALT state flag
	MEMPTR uint16 // MEMPTR register (undocumented)

	Memory Memory // Memory interface
	IO     IO     // IO interface
}

// New creates a new Z80 CPU instance
func New(memory Memory, io IO) *CPU {
	return &CPU{
		Memory: memory,
		IO:     io,
	}
}

// GetBC returns the combined value of the B and C registers
func (cpu *CPU) GetBC() uint16 {
	return (uint16(cpu.B) << 8) | uint16(cpu.C)
}

// GetDE returns the combined value of the D and E registers
func (cpu *CPU) GetDE() uint16 {
	return (uint16(cpu.D) << 8) | uint16(cpu.E)
}

// GetHL returns the combined value of the H and L registers
func (cpu *CPU) GetHL() uint16 {
	return (uint16(cpu.H) << 8) | uint16(cpu.L)
}

// SetBC sets the B and C registers from a 16-bit value
func (cpu *CPU) SetBC(value uint16) {
	cpu.B = byte(value >> 8)
	cpu.C = byte(value)
}

// SetDE sets the D and E registers from a 16-bit value
func (cpu *CPU) SetDE(value uint16) {
	cpu.D = byte(value >> 8)
	cpu.E = byte(value)
}

// SetHL sets the H and L registers from a 16-bit value
func (cpu *CPU) SetHL(value uint16) {
	cpu.H = byte(value >> 8)
	cpu.L = byte(value)
}

// GetIXH returns the high byte of the IX register
func (cpu *CPU) GetIXH() byte {
	return byte(cpu.IX >> 8)
}

// GetIXL returns the low byte of the IX register
func (cpu *CPU) GetIXL() byte {
	return byte(cpu.IX)
}

// GetIYH returns the high byte of the IY register
func (cpu *CPU) GetIYH() byte {
	return byte(cpu.IY >> 8)
}

// GetIYL returns the low byte of the IY register
func (cpu *CPU) GetIYL() byte {
	return byte(cpu.IY)
}

// SetIXH sets the high byte of the IX register
func (cpu *CPU) SetIXH(value byte) {
	cpu.IX = (cpu.IX & 0x00FF) | (uint16(value) << 8)
}

// SetIXL sets the low byte of the IX register
func (cpu *CPU) SetIXL(value byte) {
	cpu.IX = (cpu.IX & 0xFF00) | uint16(value)
}

// SetIYH sets the high byte of the IY register
func (cpu *CPU) SetIYH(value byte) {
	cpu.IY = (cpu.IY & 0x00FF) | (uint16(value) << 8)
}

// SetIYL sets the low byte of the IY register
func (cpu *CPU) SetIYL(value byte) {
	cpu.IY = (cpu.IY & 0xFF00) | uint16(value)
}

// GetFlag returns the state of a specific flag
func (cpu *CPU) GetFlag(flag byte) bool {
	return (cpu.F & flag) != 0
}

// SetFlag sets a flag to a specific state
func (cpu *CPU) SetFlag(flag byte, state bool) {
	if state {
		cpu.F |= flag
	} else {
		cpu.F &^= flag
	}
}

// SetFlagState is a helper function to set a flag based on a boolean condition
func (cpu *CPU) SetFlagState(flag byte, condition bool) {
	if condition {
		cpu.F |= flag
	} else {
		cpu.F &^= flag
	}
}

// ClearFlag clears a specific flag
func (cpu *CPU) ClearFlag(flag byte) {
	cpu.F &^= flag
}

// ClearAllFlags clears all flags
func (cpu *CPU) ClearAllFlags() {
	cpu.F = 0
}

// ReadImmediateByte reads the next byte from memory at PC and increments PC
func (cpu *CPU) ReadImmediateByte() byte {
	value := cpu.Memory.ReadByte(cpu.PC)
	cpu.PC++
	return value
}

// ReadImmediateWord reads the next word from memory at PC and increments PC by 2
func (cpu *CPU) ReadImmediateWord() uint16 {
	lo := cpu.Memory.ReadByte(cpu.PC)
	cpu.PC++
	hi := cpu.Memory.ReadByte(cpu.PC)
	cpu.PC++
	return (uint16(hi) << 8) | uint16(lo)
}

// ReadDisplacement reads an 8-bit signed displacement value
func (cpu *CPU) ReadDisplacement() int8 {
	value := int8(cpu.Memory.ReadByte(cpu.PC))
	cpu.PC++
	return value
}

// ReadOpcode reads the next opcode from memory at PC and increments PC
func (cpu *CPU) ReadOpcode() byte {
	opcode := cpu.Memory.ReadByte(cpu.PC)
	cpu.PC++
	// Increment R register (memory refresh) for each opcode fetch
	// Note: R is a 7-bit register, bit 7 remains unchanged
	cpu.R = (cpu.R & 0x80) | ((cpu.R + 1) & 0x7F)
	return opcode
}

// Push pushes a 16-bit value onto the stack
func (cpu *CPU) Push(value uint16) {
	cpu.SP -= 2
	cpu.Memory.WriteWord(cpu.SP, value)
}

// Pop pops a 16-bit value from the stack
func (cpu *CPU) Pop() uint16 {
	// Read low byte first, then high byte (little-endian)
	lo := cpu.Memory.ReadByte(cpu.SP)
	hi := cpu.Memory.ReadByte(cpu.SP + 1)
	cpu.SP += 2
	return (uint16(hi) << 8) | uint16(lo)
}

// UpdateSZFlags updates the S and Z flags based on an 8-bit result
func (cpu *CPU) UpdateSZFlags(result byte) {
	cpu.SetFlagState(FLAG_S, (result&0x80) != 0)
	cpu.SetFlagState(FLAG_Z, result == 0)
}

func (cpu *CPU) UpdatePVFlags(result byte) {
	// Calculate parity (even number of 1-bits = 1, odd = 0)
	parity := byte(1)
	for i := 0; i < 8; i++ {
		parity ^= (result >> i) & 1
	}
	cpu.SetFlagState(FLAG_PV, parity != 0)
}

// UpdateSZXYPVFlags updates the S, Z, X, Y, P/V flags based on an 8-bit result
func (cpu *CPU) UpdateSZXYPVFlags(result byte) {
	cpu.SetFlagState(FLAG_S, (result&0x80) != 0)
	cpu.SetFlagState(FLAG_Z, result == 0)
	cpu.SetFlagState(FLAG_X, (result&FLAG_X) != 0)
	cpu.SetFlagState(FLAG_Y, (result&FLAG_Y) != 0)

	// Calculate parity (even number of 1-bits = 1, odd = 0)
	parity := byte(1)
	for i := 0; i < 8; i++ {
		parity ^= (result >> i) & 1
	}
	cpu.SetFlagState(FLAG_PV, parity != 0)
}

// UpdateFlags3and5FromValue updates the X and Y flags from an 8-bit value
func (cpu *CPU) UpdateFlags3and5FromValue(value byte) {
	cpu.SetFlagState(FLAG_X, (value&FLAG_X) != 0)
	cpu.SetFlagState(FLAG_Y, (value&FLAG_Y) != 0)
}

// UpdateFlags3and5FromAddress updates the X and Y flags from the high byte of an address
func (cpu *CPU) UpdateFlags3and5FromAddress(address uint16) {
	cpu.SetFlagState(FLAG_X, (byte(address>>8)&FLAG_X) != 0)
	cpu.SetFlagState(FLAG_Y, (byte(address>>8)&FLAG_Y) != 0)
}

// UpdateSZXYFlags updates the S, Z, X, Y flags based on an 8-bit result
func (cpu *CPU) UpdateSZXYFlags(result byte) {
	cpu.SetFlagState(FLAG_S, (result&0x80) != 0)
	cpu.SetFlagState(FLAG_Z, result == 0)
	cpu.SetFlagState(FLAG_X, (result&FLAG_X) != 0)
	cpu.SetFlagState(FLAG_Y, (result&FLAG_Y) != 0)
}

// boolToByte converts a boolean to a byte (1 if true, 0 if false)
func boolToByte(b bool) byte {
	if b {
		return 1
	}
	return 0
}

// ExecuteOneInstruction executes a single instruction and returns the number of T-states used
func (cpu *CPU) ExecuteOneInstruction() int {
	// Handle interrupts first if enabled
	if cpu.IFF1 && cpu.IO.CheckInterrupt() {
		return cpu.HandleInterrupt()
	}
	//fmt.Printf("%b",cpu.HALT)
	// Handle HALT state
	if cpu.HALT {
		return 4 // 4 T-states for HALT
	}

	// Read the next opcode
	opcode := cpu.ReadOpcode()

	// Handle prefixed opcodes
	switch opcode {
	case 0xCB:
		cbOpcode := cpu.ReadOpcode()
		return cpu.ExecuteCBOpcode(cbOpcode)
	case 0xDD:
		ddOpcode := cpu.ReadOpcode()
		return cpu.ExecuteDDOpcode(ddOpcode)
	case 0xED:
		edOpcode := cpu.ReadOpcode()
		return cpu.ExecuteEDOpcode(edOpcode)
	case 0xFD:
		fdOpcode := cpu.ReadOpcode()
		return cpu.ExecuteFDOpcode(fdOpcode)
	default:
		return cpu.ExecuteOpcode(opcode)
	}
}

// HandleInterrupt handles interrupt processing
func (cpu *CPU) HandleInterrupt() int {
	// Exit HALT state
	// The HALT instruction halts the Z80; it does not increase the PC so that the instruction is re-
	//executed, until a maskable or non-maskable interrupt is accepted. Only then does the Z80 increase
	//the PC again and continues with the next instruction. During the HALT state, the HALT line is
	//set. The PC is increased before the interrupt routine is called.
	if(cpu.HALT) {
		cpu.HALT = false
		cpu.PC++
	}
	
	
	// Reset interrupt flip-flops
	cpu.IFF1 = false
	cpu.IFF2 = false
	// Handle interrupt based on mode
	switch cpu.IM {
	case 0, 1:
		// Mode 0/1: Restart at address 0x0038
		cpu.Push(cpu.PC)
		cpu.PC = 0x0038
		return 13 // 13 T-states for interrupt handling
	case 2:
		// Mode 2: Call interrupt vector
		cpu.Push(cpu.PC)
		vectorAddr := (uint16(cpu.I) << 8) | 0xFF // Use 0xFF as vector for non-maskable interrupt
		cpu.PC = cpu.Memory.ReadWord(vectorAddr)
		return 19 // 19 T-states for interrupt handling
	default:
		// Should not happen, but handle gracefully
		return 0
	}
}

// HandleNMI handles non-maskable interrupt
func (cpu *CPU) HandleNMI() int {
	// Save current PC on stack
	cpu.Push(cpu.PC)

	// Jump to NMI handler
	cpu.PC = 0x0066

	// Disable interrupts
	cpu.IFF1 = false

	return 11 // 11 T-states for NMI handling
}

// GetAF returns the combined value of the A and F registers
func (cpu *CPU) GetAF() uint16 {
	return (uint16(cpu.A) << 8) | uint16(cpu.F)
}

// SetAF sets the A and F registers from a 16-bit value
func (cpu *CPU) SetAF(value uint16) {
	cpu.A = byte(value >> 8)
	cpu.F = byte(value)
}

// GetAF_ returns the combined value of the alternate A_ and F_ registers
func (cpu *CPU) GetAF_() uint16 {
	return (uint16(cpu.A_) << 8) | uint16(cpu.F_)
}

// SetAF_ sets the alternate A_ and F_ registers from a 16-bit value
func (cpu *CPU) SetAF_(value uint16) {
	cpu.A_ = byte(value >> 8)
	cpu.F_ = byte(value)
}

// GetBC_ returns the combined value of the alternate B_ and C_ registers
func (cpu *CPU) GetBC_() uint16 {
	return (uint16(cpu.B_) << 8) | uint16(cpu.C_)
}

// GetDE_ returns the combined value of the alternate D_ and E_ registers
func (cpu *CPU) GetDE_() uint16 {
	return (uint16(cpu.D_) << 8) | uint16(cpu.E_)
}

// GetHL_ returns the combined value of the alternate H_ and L_ registers
func (cpu *CPU) GetHL_() uint16 {
	return (uint16(cpu.H_) << 8) | uint16(cpu.L_)
}

// SetBC_ sets the alternate B_ and C_ registers from a 16-bit value
func (cpu *CPU) SetBC_(value uint16) {
	cpu.B_ = byte(value >> 8)
	cpu.C_ = byte(value)
}

// SetDE_ sets the alternate D_ and E_ registers from a 16-bit value
func (cpu *CPU) SetDE_(value uint16) {
	cpu.D_ = byte(value >> 8)
	cpu.E_ = byte(value)
}

// SetHL_ sets the alternate H_ and L_ registers from a 16-bit value
func (cpu *CPU) SetHL_(value uint16) {
	cpu.H_ = byte(value >> 8)
	cpu.L_ = byte(value)
}

// UpdateXYFlags updates the undocumented X and Y flags based on an 8-bit result
func (cpu *CPU) UpdateXYFlags(result byte) {
	cpu.SetFlagState(FLAG_X, (result&FLAG_X) != 0)
	cpu.SetFlagState(FLAG_Y, (result&FLAG_Y) != 0)
}
