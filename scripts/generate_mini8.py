#!/usr/bin/env python3
"""Append the MINI8 CPU to the existing LogicCircuit project."""

import base64
import os
import sys
import tempfile
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT / "电路项目.CircuitProject"
NS = "http://LogicCircuit.net/2.0.0.14/CircuitProject.xsd"
CPU_ID = "88888888-8888-4888-8888-888888888888"
CPU_NAME = "MINI8 CPU"
LED = "00000000-0000-0000-0000-000000080100"

# Persistent GateType, InputCount, Inverted tuples encoded by GateSet.GateGuid.
GATES = {
    "NOT": ("00000000-0000-0000-0000-000000020101", 2, 1, True),
    "OR2": ("00000000-0000-0000-0000-000000030200", 3, 2, False),
    "NOR8": ("00000000-0000-0000-0000-000000030801", 3, 8, True),
    "AND2": ("00000000-0000-0000-0000-000000040200", 4, 2, False),
    "LED": (LED, 8, 1, False),
}

REG8 = "6a880594-411b-4275-a5da-d803bf8a3046"
PC8 = "22222222-2222-4222-8222-222222222222"
BUS8 = "44444444-4444-4444-8444-444444444444"
ALU8 = "fcdca2cc-aaa6-41d9-bcfc-ba7b6bbaa43e"
DFFR = "5b377d3e-a998-41c7-b6cb-02d9fa54b96f"


def gid(name):
    return str(uuid.uuid5(uuid.UUID("88aa88aa-88aa-48aa-88aa-88aa88aa88aa"), name))


def line(tag, **attrs):
    values = " ".join(f'{key}="{value}"' for key, value in attrs.items())
    return f"\t<{tag} {values} />\n"


def validate_gate_guids():
    for name, (value, gate_type, inputs, inverted) in GATES.items():
        raw = bytes.fromhex(value.replace("-", ""))
        encoded = raw[-3:]
        expected = bytes((gate_type, inputs, int(inverted)))
        if encoded != expected:
            raise ValueError(f"{name} gate GUID encodes {tuple(encoded)}, expected {tuple(expected)}")
    if GATES["LED"][0] != LED:
        raise ValueError("LED GUID regression")


def valid_builtin_gate(value):
    if not value.startswith("00000000-0000-0000-0000-"):
        return False
    gate_type, inputs, inverted = bytes.fromhex(value.replace("-", ""))[-3:]
    if inverted not in (0, 1):
        return False
    if gate_type == 1:
        return inputs == 0 and inverted == 0
    if gate_type == 2:
        return inputs == 1 and inverted == 1
    if gate_type in (3, 4, 5):
        return 2 <= inputs <= 18
    if gate_type == 8:
        return inputs in (1, 8) and inverted == 0
    if gate_type in (10, 11):
        return inputs == 2 and inverted == 0
    return False


class Builder:
    def __init__(self):
        self.definitions = []
        self.pins = []
        self.symbols = []
        self.wires = []
        self.symbol_count = 0
        self.wire_count = 0

    def definition(self, tag, **attrs):
        self.definitions.append(line(tag, **attrs))

    def pin(self, name, bit_width=1, output=False, index=0):
        pin_id = gid("pin:" + name)
        attrs = {"PinId": pin_id, "CircuitId": CPU_ID}
        if bit_width != 1:
            attrs["BitWidth"] = bit_width
        if output:
            attrs["PinType"] = "Output"
            attrs["PinSide"] = "Right"
        attrs["Name"] = name
        attrs["JamNotation"] = name
        if index:
            attrs["Index"] = index
        self.pins.append(line("Pin", **attrs))
        return pin_id

    def symbol(self, circuit_id, x, y, name, rotation=None):
        symbol_id = gid(f"symbol:{name}")
        attrs = {
            "CircuitSymbolId": symbol_id,
            "CircuitId": circuit_id,
            "LogicalCircuitId": CPU_ID,
            "X": x,
            "Y": y,
        }
        if rotation:
            attrs["Rotation"] = rotation
        self.symbols.append(line("CircuitSymbol", **attrs))
        self.symbol_count += 1
        return symbol_id

    def wire(self, p1, p2, name):
        attrs = {
            "WireId": gid(f"wire:{name}:{self.wire_count}"),
            "LogicalCircuitId": CPU_ID,
            "X1": p1[0], "Y1": p1[1], "X2": p2[0], "Y2": p2[1],
        }
        self.wires.append(line("Wire", **attrs))
        self.wire_count += 1


def control_rom():
    PC_O, MAR_L, MEM_O, IR_L = 0, 1, 2, 3
    PC_INC, IRLO_O, A_L, A_O = 4, 5, 6, 7
    ALU_O, SUB, MEM_W, PC_L, HLT_SET = 8, 9, 10, 11, 13
    words = [0] * 128

    def set_word(opcode, step, *signals, zero=None):
        word = sum(1 << signal for signal in signals)
        zero_values = (0, 1) if zero is None else (zero,)
        for z in zero_values:
            words[opcode | (step << 4) | (z << 6)] = word

    for opcode in range(16):
        set_word(opcode, 0, PC_O, MAR_L)
        set_word(opcode, 1, MEM_O, IR_L, PC_INC)
    set_word(1, 2, IRLO_O, A_L)                 # LDI
    set_word(2, 2, IRLO_O, MAR_L)               # LDA address
    set_word(2, 3, MEM_O, A_L)
    set_word(3, 2, IRLO_O, MAR_L)               # STA address
    set_word(3, 3, MEM_W)
    set_word(4, 2, IRLO_O, MAR_L)               # ADD address
    set_word(4, 3, ALU_O, A_L)
    set_word(5, 2, IRLO_O, MAR_L)               # SUB address
    set_word(5, 3, ALU_O, A_L, SUB)
    set_word(6, 2, IRLO_O, PC_L)                # JMP
    set_word(7, 2, zero=0)                      # JZ not taken
    set_word(7, 2, IRLO_O, PC_L, zero=1)        # JZ taken
    set_word(8, 2, HLT_SET)                     # HLT
    data = bytearray()
    for word in words:
        data.extend((word & 0xff, word >> 8))
    return base64.b64encode(data).decode("ascii")


def build():
    b = Builder()
    ram_id = gid("memory:ram")
    control_id = gid("memory:control")
    split_ids = {name: gid("splitter:" + name) for name in (
        "ram_out", "ram_addr", "ram_in", "control_addr", "control_out",
        "a_out", "pc_out", "mar_out", "ir_out", "bus_out", "step_out",
    )}
    zero_id = gid("constant:zero")
    one_id = gid("constant:one")
    fifteen_id = gid("constant:fifteen")

    b.definition("LogicalCircuit", LogicalCircuitId=CPU_ID, Name=CPU_NAME,
                 Notation="MINI8", Note="8-bit CPU, 16x8 unified RAM, four microsteps")

    pin_ids = {
        "CLK": b.pin("CLK"),
        "RESET": b.pin("RESET", index=1),
        "A": b.pin("A", 8, True),
        "MEM_F": b.pin("MEM_F", 8, True, 1),
        "PC": b.pin("PC", 8, True, 2),
        "MAR": b.pin("MAR", 8, True, 3),
        "IR": b.pin("IR", 8, True, 4),
        "BUS": b.pin("BUS", 8, True, 5),
        "STEP": b.pin("STEP", 2, True, 6),
        "ZERO": b.pin("ZERO", output=True, index=7),
        "HALT": b.pin("HALT", output=True, index=8),
        "CONTROL": b.pin("CONTROL", 16, True, 9),
    }

    program = bytes((0x13, 0x4e, 0x3f, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0))
    b.definition("Constant", ConstantId=zero_id)
    b.definition("Constant", ConstantId=one_id, Value=1)
    b.definition("Constant", ConstantId=fifteen_id, BitWidth=4, Value=15)
    b.definition("Memory", MemoryId=ram_id, Writable="True", WriteOn1="True",
                 OnStart="Data", AddressBitWidth=4, DataBitWidth=8,
                 DualPort="True", Data=base64.b64encode(program).decode("ascii"),
                 Note="Program: LDI 3; ADD E; STA F; HLT. Data[E]=2")
    b.definition("Memory", MemoryId=control_id, AddressBitWidth=7, DataBitWidth=16,
                 Data=control_rom(), Note="opcode[3:0], step[1:0], zero -> 16 control bits")
    for name, bit_width, pin_count, clockwise in (
        ("ram_out", 8, 8, True), ("ram_addr", 4, 4, False),
        ("ram_in", 8, 8, False), ("control_addr", 7, 7, False),
        ("control_out", 16, 16, True), ("a_out", 8, 8, False),
        ("pc_out", 8, 8, False), ("mar_out", 8, 8, False),
        ("ir_out", 8, 8, False), ("bus_out", 8, 8, False),
        ("step_out", 2, 2, False),
    ):
        b.definition("Splitter", SplitterId=split_ids[name], BitWidth=bit_width,
                     PinCount=pin_count, Clockwise=str(clockwise))

    # Core symbols. Coordinates below use the pin geometry calculated by Circuit.Update.
    b.symbol(pin_ids["CLK"], 5, 10, "pin-clk")
    b.symbol(pin_ids["RESET"], 5, 14, "pin-reset")
    output_y = {"A": 13, "MEM_F": 22, "PC": 31, "MAR": 40, "IR": 49,
                "BUS": 58, "STEP": 70, "ZERO": 75, "HALT": 79, "CONTROL": 84}
    for name, y in output_y.items():
        x = 76 if name == "MEM_F" else 140
        y = 19 if name == "MEM_F" else y
        b.symbol(pin_ids[name], x, y, "pin-" + name.lower())

    positions = {
        "pc": (20, 10), "mar": (50, 10), "ir": (50, 30), "a": (50, 50),
        "step": (80, 80), "alu": (75, 48), "ram": (70, 17),
        "ram_out": (80, 14), "ram_addr": (64, 14), "ram_in": (64, 25),
        "control_addr": (95, 10), "control_rom": (100, 12), "control_out": (108, 6),
    }
    b.symbol(PC8, *positions["pc"], "pc")
    b.symbol(REG8, *positions["mar"], "mar")
    b.symbol(REG8, *positions["ir"], "ir")
    b.symbol(REG8, *positions["a"], "a")
    b.symbol(PC8, *positions["step"], "step")
    b.symbol(ALU8, *positions["alu"], "alu")
    b.symbol(ram_id, *positions["ram"], "ram")
    for name in ("ram_out", "ram_addr", "ram_in", "control_addr", "control_out"):
        b.symbol(split_ids[name], *positions[name], name)
    b.symbol(control_id, *positions["control_rom"], "control-rom")

    driver_names = ("pc", "ram", "irlo", "a", "alu")
    driver_positions = {name: (30, 10 + i * 18) for i, name in enumerate(driver_names)}
    for name in driver_names:
        b.symbol(BUS8, *driver_positions[name], "driver-" + name)

    # Constants, load-control inverters, zero detector, and halt/clock logic.
    b.symbol(zero_id, 8, 25, "const-zero")
    b.symbol(one_id, 8, 29, "const-one")
    b.symbol(fifteen_id, 66, 19, "const-f")
    for name, pos in {"mar": (44, 18), "ir": (44, 38), "a": (44, 58)}.items():
        b.symbol(GATES["NOT"][0], *pos, "load-not-" + name)
    b.symbol(GATES["NOR8"][0], 90, 48, "zero-detector")
    b.symbol(GATES["OR2"][0], 112, 65, "halt-or")
    b.symbol(DFFR, 120, 65, "halt-ff")
    b.symbol(GATES["NOT"][0], 110, 75, "halt-not")
    b.symbol(GATES["AND2"][0], 120, 75, "clock-and")

    status_split_positions = {
        "a_out": (130, 10), "pc_out": (130, 28), "mar_out": (130, 37),
        "ir_out": (130, 46), "bus_out": (130, 55), "step_out": (130, 69),
    }
    for name, pos in status_split_positions.items():
        b.symbol(split_ids[name], *pos, "status-" + name)
    b.symbol(GATES["LED"][0], 134, 74, "led-zero")
    b.symbol(GATES["LED"][0], 134, 78, "led-halt")
    for i in range(8):
        b.symbol(GATES["LED"][0], 124, 10 + i * 2, f"led-a{i}")

    zero = (10, 26)
    one = (10, 30)
    clk = (7, 11)
    reset = (7, 15)
    gated_clk = (123, 77)

    # RAM ports and monitor port.
    b.wire((73, 18), (80, 18), "ram-dataout-wide")
    b.wire((65, 16), (70, 18), "ram-address-wide")
    b.wire((65, 29), (70, 19), "ram-datain-wide")
    b.wire((68, 20), (70, 20), "ram-monitor-address-f")
    b.wire((73, 20), (76, 20), "ram-monitor-output")

    # Control ROM address: opcode low-to-high, step, then zero.
    ir_q = [(53, 31 + i) for i in range(8)]
    pc_q = [(29, 11 + i) for i in range(8)]
    mar_q = [(53, 11 + i) for i in range(8)]
    a_q = [(53, 51 + i) for i in range(8)]
    step_q = [(89, 81 + i) for i in range(8)]
    ram_q = [(81, 15 + i) for i in range(8)]
    alu_r = [(84, 49 + i) for i in range(8)]
    zero_signal = (93, 52)
    for i in range(4):
        b.wire(ir_q[4 + i], (95, 11 + i), f"opcode-{i}")
    b.wire(step_q[0], (95, 15), "step-address-0")
    b.wire(step_q[1], (95, 16), "step-address-1")
    b.wire(zero_signal, (95, 17), "zero-address")
    b.wire((96, 14), (100, 14), "control-address-wide")
    b.wire((103, 14), (108, 14), "control-word-wide")
    b.wire((103, 14), (140, 85), "control-status")
    control = [(109, 7 + i) for i in range(16)]

    # Register loads are active low; all clocks stop after HALT except the halt FF.
    for source, inverter_in, inverter_out, load in (
        (control[1], (44, 20), (47, 20), (50, 21)),
        (control[3], (44, 40), (47, 40), (50, 41)),
        (control[6], (44, 60), (47, 60), (50, 61)),
    ):
        b.wire(source, inverter_in, "load-control")
        b.wire(inverter_out, load, "load-active-low")
    for point in ((20, 15), (50, 20), (50, 40), (50, 60), (80, 85)):
        b.wire(gated_clk, point, "gated-clock")
    for point in ((20, 17), (50, 19), (50, 39), (50, 59), (80, 87)):
        b.wire(reset, point, "reset")
    b.wire(control[11], (20, 11), "pc-load")
    b.wire(control[4], (20, 13), "pc-increment")
    b.wire(zero, (80, 81), "step-load-off")
    b.wire(one, (80, 83), "step-increment")

    # ALU gets A directly and RAM's combinational output as B.
    for i in range(8):
        b.wire(a_q[i], (75, 50 + i), f"alu-a-{i}")
        b.wire(ram_q[i], (76 + i, 60), f"alu-b-{i}")
        b.wire(a_q[i], (90, 49 + i), f"zero-test-{i}")
    b.wire(control[9], (75, 49), "alu-sub")

    # Source drivers feed the single eight-bit tri-state bus, represented as 8 conductors.
    driver_data = {"pc": pc_q, "ram": ram_q, "irlo": ir_q[:4] + [zero] * 4,
                   "a": a_q, "alu": alu_r}
    driver_enable = {"pc": control[0], "ram": control[2], "irlo": control[5],
                     "a": control[7], "alu": control[8]}
    bus_hubs = [(105 + i * 3, 112) for i in range(8)]
    for name in driver_names:
        x, y = driver_positions[name]
        for i, source in enumerate(driver_data[name]):
            b.wire(source, (x, y + 1 + i), f"{name}-driver-in-{i}")
            b.wire((x + 3, y + 1 + i), bus_hubs[i], f"{name}-bus-{i}")
        b.wire(driver_enable[name], (x, y + 9), f"{name}-enable")

    # Bus sinks: PC, MAR, IR, A, RAM data input, and visible BUS output.
    for i, hub in enumerate(bus_hubs):
        for point, name in (
            ((21 + i, 19), "pc-d"), ((50, 11 + i), "mar-d"),
            ((50, 31 + i), "ir-d"), ((50, 51 + i), "a-d"),
            ((130, 56 + i), "bus-status"),
        ):
            b.wire(hub, point, f"{name}-{i}")
        b.wire(a_q[i], (64, 26 + i), f"ram-write-data-{i}")
    b.wire(control[10], (71, 21), "ram-write")

    # MAR low nibble selects unified RAM.
    for i in range(4):
        b.wire(mar_q[i], (64, 15 + i), f"ram-address-{i}")

    # Sticky HALT and clock gating.
    halt_q = (123, 67)
    b.wire(halt_q, (112, 66), "halt-feedback")
    b.wire(control[13], (112, 68), "halt-set")
    b.wire((115, 67), (120, 66), "halt-d")
    b.wire(clk, (120, 67), "halt-clock")
    b.wire(reset, (120, 68), "halt-reset")
    b.wire(halt_q, (110, 77), "halt-invert")
    b.wire((113, 77), (120, 76), "run-clock-enable")
    b.wire(clk, (120, 78), "raw-clock")

    # Status combiners and panel indicators.
    status_sources = {"a_out": a_q, "pc_out": pc_q, "mar_out": mar_q,
                      "ir_out": ir_q, "bus_out": bus_hubs}
    status_outputs = {"a_out": (140, 14), "pc_out": (140, 32),
                      "mar_out": (140, 41), "ir_out": (140, 50),
                      "bus_out": (140, 59)}
    for name, sources in status_sources.items():
        x, y = status_split_positions[name]
        for i, source in enumerate(sources):
            b.wire(source, (x, y + 1 + i), f"status-{name}-{i}")
        b.wire((x + 1, y + 4), status_outputs[name], f"status-{name}-wide")
    b.wire(step_q[0], (130, 70), "step-status-0")
    b.wire(step_q[1], (130, 71), "step-status-1")
    b.wire((131, 70), (140, 71), "step-status-wide")
    b.wire(zero_signal, (140, 76), "zero-output")
    b.wire(zero_signal, (134, 75), "zero-led")
    b.wire(halt_q, (140, 80), "halt-output")
    b.wire(halt_q, (134, 79), "halt-led")
    for i in range(8):
        b.wire(a_q[i], (124, 11 + i * 2), f"a-led-{i}")

    return b


def validate_xml(text):
    root = ET.fromstring(text)
    q = lambda name: f"{{{NS}}}{name}"
    ids = {}
    for element in root:
        for key, value in element.attrib.items():
            if key.endswith("Id") and key not in ("CircuitId", "LogicalCircuitId", "StartupCircuitId"):
                if value in ids:
                    raise ValueError(f"duplicate GUID {value}: {ids[value]} and {element.tag}")
                ids[value] = element.tag
    logical_ids = {e.attrib["LogicalCircuitId"] for e in root.findall(q("LogicalCircuit"))}
    circuit_ids = set(logical_ids)
    for tag, attr in (("Pin", "PinId"), ("Constant", "ConstantId"),
                      ("CircuitButton", "CircuitButtonId"), ("Memory", "MemoryId"),
                      ("Splitter", "SplitterId"), ("CircuitProbe", "CircuitProbeId")):
        circuit_ids.update(e.attrib[attr] for e in root.findall(q(tag)))
    builtin = {item[0] for item in GATES.values()}
    for symbol in root.findall(q("CircuitSymbol")):
        owner = symbol.attrib["LogicalCircuitId"]
        target = symbol.attrib["CircuitId"]
        if owner not in logical_ids:
            raise ValueError(f"CircuitSymbol owner FK missing: {owner}")
        if target not in circuit_ids and target not in builtin:
            # Other pre-existing built-in gates are validated by their encoded GUID.
            if valid_builtin_gate(target):
                continue
            raise ValueError(f"CircuitSymbol circuit FK missing: {target}")
    for wire in root.findall(q("Wire")):
        if wire.attrib["LogicalCircuitId"] not in logical_ids:
            raise ValueError("Wire owner FK missing")
    if not any(e.attrib.get("LogicalCircuitId") == CPU_ID for e in root.findall(q("LogicalCircuit"))):
        raise ValueError("MINI8 circuit missing after generation")


def insert_before(text, marker, addition):
    index = text.find(marker)
    if index < 0:
        raise ValueError(f"insertion marker not found: {marker}")
    return text[:index] + "".join(addition) + text[index:]


def main():
    validate_gate_guids()
    original = PROJECT.read_text(encoding="utf-8-sig")
    root = ET.fromstring(original)
    q = f"{{{NS}}}LogicalCircuit"
    exists = any(e.attrib.get("LogicalCircuitId") == CPU_ID or e.attrib.get("Name") == CPU_NAME
                 for e in root.findall(q))
    rebuild = len(sys.argv) == 2 and sys.argv[1] == "--rebuild"
    if exists and not rebuild:
        print("MINI8 CPU already exists; no changes made")
        return 0
    b = build()
    if exists:
        generated_ids = {gid("memory:ram"), gid("memory:control"), gid("constant:zero"),
                         gid("constant:one"), gid("constant:fifteen")}
        generated_ids.update(gid("splitter:" + name) for name in (
            "ram_out", "ram_addr", "ram_in", "control_addr", "control_out",
            "a_out", "pc_out", "mar_out", "ir_out", "bus_out", "step_out",
        ))
        original = "".join(
            item for item in original.splitlines(keepends=True)
            if CPU_ID not in item and not any(value in item for value in generated_ids)
        )
    generated = insert_before(original, "\t<Pin ", b.definitions[:1])
    generated = insert_before(generated, "\t<Constant ", b.pins)
    generated = insert_before(generated, "\t<CircuitSymbol ", b.definitions[1:] + b.symbols)
    generated = insert_before(generated, "\t<Wire ", b.wires)
    validate_xml(generated)

    fd, temp_name = tempfile.mkstemp(prefix=PROJECT.name + ".", suffix=".tmp", dir=PROJECT.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as output:
            output.write(generated)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temp_name, PROJECT)
    finally:
        if os.path.exists(temp_name):
            os.unlink(temp_name)
    print(f"Added {CPU_NAME}: {b.symbol_count} symbols, {b.wire_count} wires")
    return 0


if __name__ == "__main__":
    sys.exit(main())
